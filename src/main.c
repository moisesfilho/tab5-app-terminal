/**
 * @file main.c
 * @brief Aplicativo Terminal / Shell para Tab5 OS
 */

#include "tab5_sdk.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char s_terminal_history[2048] = {0};

static void append_terminal_line(const char *line)
{
    size_t cur_len = strlen(s_terminal_history);
    size_t line_len = strlen(line);
    if (cur_len + line_len + 2 < sizeof(s_terminal_history)) {
        strcat(s_terminal_history, line);
        strcat(s_terminal_history, "\n");
    } else {
        /* Truncar inicio do historico */
        snprintf(s_terminal_history, sizeof(s_terminal_history), "[...]\n%s\n", line);
    }
    tab5_ui_obj_t ta = tab5_ui_get_main_textarea();
    if (ta != NULL) {
        tab5_ui_textarea_set_text(ta, s_terminal_history);
    }
}

static void on_clear_terminal(void *user_data)
{
    (void)user_data;
    s_terminal_history[0] = '\0';
    append_terminal_line("Tab5 OS Micro-Shell [v1.0.0]");
    append_terminal_line("Digite 'help' para ver os comandos disponiveis.");
    append_terminal_line("-------------------------------------------");
    tab5_sound_play_beep(1000, 20);
    tab5_ui_show_toast("Terminal limpo", 1200);
}

static void on_exec_help(void *user_data)
{
    (void)user_data;
    append_terminal_line("> help");
    append_terminal_line("Comandos integrados:");
    append_terminal_line("  help      - Exibe esta mensagem de ajuda");
    append_terminal_line("  sysinfo   - Exibe status de memoria e bateria");
    append_terminal_line("  wifi      - Exibe status da conexao de rede");
    append_terminal_line("  storage   - Lista caminhos de armazenamento");
    append_terminal_line("  clear     - Limpa o buffer da tela");
    tab5_sound_play_beep(1200, 25);
}

static void app_init(void)
{
    tab5_system_log(2, "tab5_terminal", "Aplicativo Terminal iniciado");
    tab5_ui_app_bar_set_title("Terminal");
    tab5_ui_app_bar_add_action_button("LV_SYMBOL_TRASH", on_clear_terminal, NULL);
    tab5_ui_app_bar_add_action_button("LV_SYMBOL_LIST", on_exec_help, NULL);

    s_terminal_history[0] = '\0';
    append_terminal_line("===========================================");
    append_terminal_line("       Tab5 OS Micro-Shell [v1.0.0]       ");
    append_terminal_line("===========================================");
    append_terminal_line("Ambiente de execucao seguro WebAssembly.");
    append_terminal_line("Pronto. Pressione 'Ajuda' para lista de comandos.\n");
}

static void app_resume(void)
{
    tab5_system_log(2, "tab5_terminal", "Terminal retomado");
}

static void app_pause(void)
{
    tab5_system_log(2, "tab5_terminal", "Terminal pausado");
}

static void app_destroy(void)
{
    tab5_system_log(2, "tab5_terminal", "Terminal finalizado");
}

TAB5_APP_EXPORT int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    tab5_lifecycle_callbacks_t cbs = {
        .on_init = app_init,
        .on_resume = app_resume,
        .on_pause = app_pause,
        .on_destroy = app_destroy,
        .on_open_file = NULL,
    };

    tab5_lifecycle_register(&cbs);
    app_init();
    return 0;
}
