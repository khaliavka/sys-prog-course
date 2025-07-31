#include "main.h"
#include "gui.h"
#include "nav.h"

int init_state(struct state *st)
{
    st->focus_index = 0;
    for (int i = 0; i < NPANELS; ++i)
    {
        st->panels[i].raw_path = malloc(PATH_MAX);
        st->panels[i].resolved_path = malloc(PATH_MAX);
        st->panels[i].prev_path = malloc(PATH_MAX);

        getcwd(st->panels[i].resolved_path, PATH_MAX);
        strncpy(st->panels[i].prev_path, st->panels[i].resolved_path, PATH_MAX);
        st->panels[i].sz = scandir(st->panels[i].resolved_path, &st->panels[i].namelist, filter_self, category_alphasort);
        st->panels[i].top = 0;
        st->panels[i].selected = 0;
    }
    return 0;
}

int free_namelist(struct dirent **namelist, int sz)
{
    for (int i = 0; i < sz; ++i)
    {
        free(namelist[i]);
    }
    return 0;
}

void cleanup(struct state *st)
{
    for (int i = 0; i < NPANELS; ++i)
    {
        delwin(st->panels[i].panel);
        delwin(st->panels[i].container);

        free(st->panels[i].prev_path);
        free(st->panels[i].raw_path);
        free(st->panels[i].resolved_path);
        free_namelist(st->panels[i].namelist, st->panels[i].sz);
    }
    endwin();
}

int main(void)
{

    struct state st;
    init_gui(&st, sig_winch);
    init_state(&st);
    render_all(&st);
    int ch;
    while ((ch = getch()) != '\e' && ch != 'q')
    {
        switch (ch)
        {
        case KEY_UP:
            up_one_line(&st.panels[st.focus_index]);
            break;

        case KEY_DOWN:
            down_one_line(&st.panels[st.focus_index]);
            break;

        case '\t':
            switch_focus(&st);
            break;

        case '\n':
            enter_dir(&st.panels[st.focus_index]);
            break;

        case KEY_HOME:
            go_up(&st.panels[st.focus_index]);
            break;

        case KEY_END:
            go_down(&st.panels[st.focus_index]);
            break;

        case KEY_BACKSPACE:
            go_up(&st.panels[st.focus_index]);
            enter_dir(&st.panels[st.focus_index]);
            break;

        case KEY_PPAGE:
            page_up(&st.panels[st.focus_index]);
            break;

        case KEY_NPAGE:
            page_down(&st.panels[st.focus_index]);
            break;

        default:
            break;
        }
        render_all(&st);
    }

    cleanup(&st);

    return EXIT_SUCCESS;
}
