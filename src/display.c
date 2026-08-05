#include <ncurses.h>
#include <panel.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include "sysmonitor.h"
#include "display.h"

struct Display {
    int maxy, maxx;

    WINDOW *mem_win;
    WINDOW *cpu_win;
    WINDOW *net_win;
    WINDOW *disk_win;

    WINDOW *proc_win;
    WINDOW *proc_pad;
    int pad_w;
    int pad_h;
    int view_h;
    int view_w;

    int top;        // offset visible row (on top)
    int cursor;     // selected row
    int procs_qty;  // current rows number
};

static void draw_box_title(WINDOW *win, const char *title, int y, int w) {
    int len = (int)strlen(title);
    int x = (w - len) / 2;
    if (x < 1) x = 1;

    mvwprintw(win, y, x, "%s", title);
    for (int i = 1; i < w - 1; i++) {
        if (y + 1 < getmaxy(win)) mvwprintw(win, y + 1, i, "-");
    }
}

Display* display_create(void) {
    Display *d = calloc(1, sizeof(Display));
    if (!d) return NULL;

    initscr();
    cbreak();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);  // Enables special keys (arrow keys, etc.)
    nodelay(stdscr, TRUE); // Non-blocking read

    getmaxyx(stdscr, d->maxy, d->maxx);

    // MEMORY
    int mem_h = d->maxy/4; if (mem_h < 4) mem_h = 4;
    int mem_w = d->maxx/6; if (mem_w < 20) mem_w = 20;
    d->mem_win = newwin(mem_h, mem_w, 1, 1);

    // CPU
    int cpu_h = d->maxy/4; if (cpu_h < 4) cpu_h = 4;
    int cpu_w = d->maxx/8; if (cpu_w < 20) cpu_w = 20;
    d->cpu_win = newwin(cpu_h, cpu_w, 1, 1 + mem_w + 4);

    // NETWORK
    int net_h = d->maxy/4; if (net_h < 4) net_h = 4;
    int net_w = d->maxx/4; if (net_w < 20) net_w = 20;
    d->net_win = newwin(net_h, net_w, 1, 1 + mem_w + 4 + cpu_w + 3);

    // DISK
int top_h = mem_h;
if (cpu_h > top_h) top_h = cpu_h;
if (net_h > top_h) top_h = net_h;

int x_start = 1;
int disk_w = mem_w + 4 + cpu_w + 3 + net_w;
if (disk_w < 56) disk_w = 56;

int disk_h = d->maxy/4; if (disk_h < 7) disk_h = 7;
d->disk_win = newwin(disk_h, disk_w, 1 + top_h, x_start);

int proc_y = 1 + top_h + disk_h + 1;
int proc_h = d->maxy - (proc_y + 1);
if (proc_h < 4) proc_h = 4;

int proc_w = disk_w;
d->proc_win = newwin(proc_h, proc_w, proc_y, x_start);


    // PAD for scrolling
    d->proc_pad = NULL;
    d->pad_w = proc_w - 2;
    d->pad_h = 1;
    d->view_h = proc_h - 2;
    d->view_w = proc_w - 2;
    d->top = 0;
    d->cursor = 0;
    d->procs_qty = 0;

    // Box
    WINDOW *windows[] = {d->mem_win, d->cpu_win, d->net_win, d->disk_win, d->proc_win};
    for (int i = 0; i < 5; i++) {
        WINDOW *w = windows[i];
        if (!w) continue;
        werase(w);
        box(w, 0, 0);
        wrefresh(w);
    }

    // Titles
    draw_box_title(d->mem_win, "MEMORY", 1, mem_w);
    draw_box_title(d->cpu_win, "CPUs USAGE", 1, cpu_w);
    draw_box_title(d->net_win, "NETWORK", 1, net_w);
    draw_box_title(d->proc_win, "PROCESSES", 1, proc_w);
    draw_box_title(d->disk_win, "DISK", 1, disk_w);

    refresh();
    return d;
}

static void update_memory(Display *d) {
    mem_stat_t mem;
    parse_mem(&mem);

    char buf[64];
    werase(d->mem_win);
    box(d->mem_win, 0, 0);

    int mem_w = getmaxx(d->mem_win);
    mvwprintw(d->mem_win, 1, (mem_w - (int)strlen("MEMORY"))/2, "MEMORY");
    for (int i = 1; i < mem_w-1; i++) mvwprintw(d->mem_win, 2, i, "-");

    human_readable(mem.mem_total * 1024ULL, buf, sizeof buf);
    mvwprintw(d->mem_win, 4, 1, " Total      %-12s", buf);

    human_readable(mem.mem_free * 1024ULL, buf, sizeof buf);
    mvwprintw(d->mem_win, 5, 1, " Free       %-12s", buf);

    human_readable(mem.mem_available * 1024ULL, buf, sizeof buf);
    mvwprintw(d->mem_win, 6, 1, " Available  %-12s", buf);

    wrefresh(d->mem_win);
}

static void update_cpu(Display *d,
                       cpu_stat_t *stats1,
                       cpu_stat_t *stats2,
                       int cpus_qty)
{
    int win_h = getmaxy(d->cpu_win);
    int win_w = getmaxx(d->cpu_win);

    const int first_row = 4;      // first useful row

    werase(d->cpu_win);
    box(d->cpu_win, 0, 0);
    draw_box_title(d->cpu_win, "CPUs USAGE", 1, win_w);

    /* necessary column width */
    int col_width = snprintf(NULL, 0,
                             " CPU #%d 100.0%%",
                             cpus_qty - 1) + 2;

    /* available width in the box */
    int usable_w = win_w - 2;

    /* Columns that fit */
    int cols = usable_w / col_width;
    if (cols < 1)
        cols = 1;

    /* Number of needed rows */
    int rows = (cpus_qty + cols - 1) / cols;

    /* If rows don't fit vertically, limits rows number */
    int max_rows = win_h - first_row - 1;
    if (rows > max_rows)
        rows = max_rows;

    for (int cpu_n = 0; cpu_n < cpus_qty; cpu_n++) {

        cpu_stat_tot_idle_t *s1 = sum_total(stats1 + cpu_n);
        cpu_stat_tot_idle_t *s2 = sum_total(stats2 + cpu_n);

        double usage = total_CPU_time(s1, s2);

        free(s1);
        free(s2);

        int col = cpu_n / rows;
        int row = cpu_n % rows;

        int x = 1 + col * col_width;
        int y = first_row + row;

        /* Avoid exiting the window */
        if (x + col_width >= win_w - 1)
            break;

        mvwprintw(d->cpu_win,
                  y,
                  x,
                  " CPU #%-2d %5.1f%%",
                  cpu_n,
                  usage);
    }

    wrefresh(d->cpu_win);
}
static void update_network(Display *d, net_v_t *n1, net_v_t *n2) {
    computed_net_bytes_t *bytes = compute_used_bandwidth(n1, n2);

    werase(d->net_win);
    box(d->net_win, 0, 0);

    int net_w = getmaxx(d->net_win);
    mvwprintw(d->net_win, 1, (net_w - (int)strlen("NETWORK"))/2, "NETWORK");
    for (int i = 1; i < net_w-1; i++) mvwprintw(d->net_win, 2, i, "-");

    mvwprintw(d->net_win, 4, 1, " Recevied bps      %.2f", bytes->received_per_sec);
    mvwprintw(d->net_win, 5, 1, " Sent bps          %.2f", bytes->sent_per_sec);
    mvwprintw(d->net_win, 6, 1, " Received bytes    %-12llu", bytes->tot_received);
    mvwprintw(d->net_win, 7, 1, " Sent     bytes    %-12llu", bytes->tot_sent);

    wrefresh(d->net_win);
}

static void update_disk(Display *d) {
    char buf[64];
    werase(d->disk_win);
    box(d->disk_win, ACS_VLINE, ACS_HLINE);

    int disk_w = getmaxx(d->disk_win);
    
    for (int i = 1; i < disk_w-1; i++) mvwprintw(d->disk_win, 2, i, "-");

    mvwprintw(d->disk_win, 3, 1, " Mount point");
    mvwprintw(d->disk_win, 3, 30, " Total");
    mvwprintw(d->disk_win, 3, 44, " Free");
    mvwprintw(d->disk_win, 3, 58, " Used");

    disks_container* fs = retrieve_fs();
    if (!fs) return;

    disk_choice choice = compute_choice_for_fstype(fs, "ext4");
    disk_space disk_bytes = choice.space;

    mvwprintw(d->disk_win, 5, 1, " %-20s", choice.mount_point ? choice.mount_point : "N/A");
    
    human_readable(disk_bytes.total, buf, sizeof buf);
    mvwprintw(d->disk_win, 5, 30, "%-12s", buf);

    human_readable(disk_bytes.free, buf, sizeof buf);
    mvwprintw(d->disk_win, 5, 44, "%-12s", buf);

    human_readable(disk_bytes.used, buf, sizeof buf);
    mvwprintw(d->disk_win, 5, 58, "%-12s", buf);

    choice = compute_choice_for_fstype(fs, "vfat");
    disk_bytes = choice.space;

    mvwprintw(d->disk_win, 6, 1, " %-20s", choice.mount_point ? choice.mount_point : "N/A");
    human_readable(disk_bytes.total, buf, sizeof buf);
    mvwprintw(d->disk_win, 6, 30, "%-10s", buf);

    human_readable(disk_bytes.free, buf, sizeof buf);
    mvwprintw(d->disk_win, 6, 44, "%-10s", buf);

    human_readable(disk_bytes.used, buf, sizeof buf);
    mvwprintw(d->disk_win, 6, 58, "%-10s", buf);

    wrefresh(d->disk_win);
    free(fs->v);
    free(fs);
}

static void update_processes(Display *d) {
    proc_v_t *procs_vector = retrieve_processes();
    if (!procs_vector) return;

    d->procs_qty = (int)procs_vector->qty;
    mvwprintw(d->proc_win, 3, 3, "Total processes: %d", d->procs_qty);
    wrefresh(d->proc_win);


    // pad height
    int needed_h = d->procs_qty + 6; // leave margin
    if (needed_h < 10) needed_h = 10;

    if (!d->proc_pad) {
        d->pad_h = needed_h;
        d->proc_pad = newpad(d->pad_h, d->pad_w);
    } else if (needed_h != d->pad_h) {
        delwin(d->proc_pad);
        d->pad_h = needed_h;
        d->proc_pad = newpad(d->pad_h, d->pad_w);
    }

    int max_top = d->procs_qty - d->view_h;
    if (max_top < 0) max_top = 0;

    if (d->top > max_top) d->top = max_top;
    if (d->cursor >= d->procs_qty) d->cursor = d->procs_qty - 1;
    if (d->cursor < 0) d->cursor = 0;

    // ----- pad design -----
    werase(d->proc_pad);

    if (getmaxy(d->proc_pad) >= 4) {
        mvwprintw(d->proc_pad, 4, 1, " PID     State         Name ");
    }

    // Truncates name to avoid overflowing the viewport.
    // Use of view_w to avoid phase shifts.
    int start_col = 1;
    int name_start = 1 + 5 + 3 + 10 + 4;
    int max_name = d->view_w - (name_start - start_col);
    if (max_name < 1) max_name = 1;

    for (size_t i = 0; i < procs_vector->qty; i++) {
        int row = (int)i + 5; 
        if (row >= getmaxy(d->proc_pad)) break;

        if ((int)i == d->cursor) wattron(d->proc_pad, A_REVERSE);

        const char *state = procs_vector->ps[i].state;
        const char *name  = procs_vector->ps[i].name;

        char namebuf[128];
        snprintf(namebuf, sizeof namebuf, "%.*s", max_name, name);

        mvwprintw(d->proc_pad, row, 1,
                  " %-5d   %-10s    %-*s",
                  (int)procs_vector->ps[i].pid,
                  state,
                  max_name, namebuf);

        if ((int)i == d->cursor) wattroff(d->proc_pad, A_REVERSE);
    }

    // Computes inner rectangle of proc_win and viewport.
    int p_begy = getbegy(d->proc_win);
    int p_begx = getbegx(d->proc_win);
    int p_maxy = getmaxy(d->proc_win);
    int p_maxx = getmaxx(d->proc_win);

    // Inner box area (without borders)
    int srow  = p_begy + 1 + 2;      // +1 border, +2 due to the title (row 1) and dashes (row 2)
    int scol  = p_begx + 1;          // after the left border
    int erow  = srow + d->view_h - 1;
    int ecol  = scol + d->view_w - 1;

    if (d->proc_pad == NULL) return;

    int ok_area = (erow >= srow) && (ecol >= scol);
    if (!ok_area) return;

    // Inner row (srow) = border+1
    // Title row 1 + dashes row 2 -> First available row to display processes is srow + 2
    srow = srow + 2;
    erow = srow + d->view_h - 1;

    // Clamp
    if (erow > p_begy + p_maxy - 2) erow = p_begy + p_maxy - 2;
    if (ecol > p_begx + p_maxx - 2) ecol = p_begx + p_maxx - 2;

    int pad_view_start = 4 + d->top;  // pad row 4 = first processes row

    prefresh(d->proc_pad,
         pad_view_start, 0,
         srow, scol,
         erow, ecol);

    wrefresh(d->proc_win);
    free_processes(procs_vector);
}

void display_update(Display *d) {
    static int inited = 0;

    static cpu_stat_t stats_prev[MAX_CPUS];
    static cpu_stat_t stats_new[MAX_CPUS];
    static int cpus_qty = 0;

    static net_v_t *net_prev = NULL;
    static net_v_t *net_new = NULL;

    static time_t last_cpu_update = 0;
    static time_t last_net_update = 0;

    if (!inited) {
        cpus_qty = parse_stats(stats_prev);
        net_prev = net_sample();

        // Starts immediately
        last_cpu_update = time(NULL) - 2;
        last_net_update = time(NULL) - 2;

        inited = 1;
    }

    
    time_t now = time(NULL);

    // CPU every 2 seconds
    if (now - last_cpu_update >= 2) {
        parse_stats(stats_new);
        update_cpu(d, stats_prev, stats_new, cpus_qty);
        memcpy(stats_prev, stats_new,
               sizeof(cpu_stat_t) * cpus_qty);
        last_cpu_update = now;
    }

    // NETWORK every 2 seconds
    if (now - last_net_update >= 2) {
        net_new = net_sample();
        update_network(d, net_prev, net_new);
        free_net(net_prev);
        net_prev = net_new;
        net_new = NULL;
        last_net_update = now;
    }

    // MEMORY
    update_memory(d);

    // CPU
    box(d->cpu_win, 0, 0);

    int cpu_w = getmaxx(d->cpu_win);
    mvwprintw(d->cpu_win, 1, (cpu_w - (int)strlen("CPUs USAGE"))/2, "CPUs USAGE");
    for (int i = 1; i < cpu_w-1; i++) mvwprintw(d->cpu_win, 2, i, "-");

    now = time(NULL);
    if (last_cpu_update == 0 || (now - last_cpu_update) >= 2) {
        parse_stats(stats_new);
        update_cpu(d, stats_prev, stats_new, cpus_qty);
        memcpy(stats_prev, stats_new, sizeof(cpu_stat_t) * cpus_qty);
        last_cpu_update = now;
    }

    // DISK
    update_disk(d);

    // PROCESSES: handles input for scrolling
    werase(d->proc_win);
    box(d->proc_win, 0, 0);
    // Title and dashes - in the box
    int proc_w = getmaxx(d->proc_win);
    mvwprintw(d->proc_win, 1, (proc_w - (int)strlen("PROCESSES"))/2, "PROCESSES");
    for (int i = 1; i < proc_w - 1; i++) {
        mvwprintw(d->proc_win, 2, i, "-");
    }
    // Input scroll
    int ch = wgetch(stdscr);

    int max_top = d->procs_qty - d->view_h;
        if (max_top < 0) max_top = 0;

    switch (ch) {
        case KEY_UP:
            if (d->cursor > 0) d->cursor--;
            if (d->top > d->cursor) d->top = d->cursor;
            break;
        case KEY_DOWN:
            if (d->cursor < d->procs_qty - 1) d->cursor++;
            if (d->top + d->view_h <= d->cursor + 1) d->top = d->cursor - d->view_h + 1;
            break;
        case KEY_PPAGE:
            d->top -= d->view_h;
            if (d->top < 0) d->top = 0;
            d->cursor = d->top;
            break;
        case KEY_NPAGE:
            d->top += d->view_h;
            if (d->top > max_top) d->top = max_top;
            d->cursor = d->top + d->view_h - 1;
            if (d->cursor >= d->procs_qty) d->cursor = d->procs_qty - 1;
            break;
        case KEY_HOME:
            d->top = 0;
            d->cursor = 0;
            break;
        case KEY_END:
            d->top = max_top;
            d->cursor = d->procs_qty - 1;
            break;
        default:
            break;
    }  
    update_processes(d);
}

void display_destroy(Display *d) {
    if (!d) return;
    if (d->proc_pad) delwin(d->proc_pad);
    if (d->mem_win) delwin(d->mem_win);
    if (d->cpu_win) delwin(d->cpu_win);
    if (d->net_win) delwin(d->net_win);
    if (d->disk_win) delwin(d->disk_win);
    if (d->proc_win) delwin(d->proc_win);
    endwin();
    free(d);
}