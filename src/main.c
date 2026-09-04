/**
 * @file main.c
 * @brief Aplicativo Terminal / Shell Desacoplado para Tab5 OS
 */

#include "tab5_sdk.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char s_term_history[4096] = {0};
static size_t s_prompt_min_index = 0;
static tab5_ui_obj_t s_ta = TAB5_UI_INVALID_OBJ;
static bool s_is_processing_cmd = false;
static char s_current_cwd[64] = "/sdcard";

static void execute_current_command(void)
{
    if (s_ta == TAB5_UI_INVALID_OBJ || s_is_processing_cmd) {
        return;
    }

    const char *full_text = tab5_ui_textarea_get_text(s_ta);
    if (full_text == NULL) {
        return;
    }

    size_t full_len = strlen(full_text);
    char input_line[256] = {0};
    if (full_len >= s_prompt_min_index) {
        strncpy(input_line, full_text + s_prompt_min_index, sizeof(input_line) - 1);
    }

    /* Remove eventuais newlines/quebras do comando digitado */
    size_t in_len = strlen(input_line);
    while (in_len > 0 && (input_line[in_len - 1] == '\n' || input_line[in_len - 1] == '\r')) {
        input_line[--in_len] = '\0';
    }

    if (strcmp(input_line, "clear") == 0) {
        s_term_history[0] = '\0';
        strcat(s_term_history, s_current_cwd);
        strcat(s_term_history, " $ ");
        s_prompt_min_index = strlen(s_term_history);
        s_is_processing_cmd = true;
        tab5_ui_textarea_set_text(s_ta, s_term_history);
        tab5_ui_textarea_set_cursor_pos(s_ta, TAB5_UI_CURSOR_LAST);
        s_is_processing_cmd = false;
        return;
    }

    /* Adiciona o comando no histórico com quebra de linha */
    if (strlen(s_term_history) + strlen(input_line) + 64 > sizeof(s_term_history)) {
        char temp[2048];
        snprintf(temp, sizeof(temp), "[... historico truncado ...]\n%s $ %s\n", s_current_cwd, input_line);
        strncpy(s_term_history, temp, sizeof(s_term_history) - 1);
    } else {
        strcat(s_term_history, input_line);
        strcat(s_term_history, "\n");
    }

    if (input_line[0] != '\0') {
        char out_buf[1024] = {0};
        tab5_err_t err = tab5_terminal_exec(input_line, out_buf, sizeof(out_buf));
        if (err == TAB5_OK && out_buf[0] != '\0') {
            if (strlen(s_term_history) + strlen(out_buf) + 64 < sizeof(s_term_history)) {
                strcat(s_term_history, out_buf);
                if (s_term_history[strlen(s_term_history) - 1] != '\n') {
                    strcat(s_term_history, "\n");
                }
            }
        } else if (err != TAB5_OK) {
            strcat(s_term_history, "Erro ao executar comando.\n");
        }
    }

    strcat(s_term_history, s_current_cwd);
    strcat(s_term_history, " $ ");
    s_prompt_min_index = strlen(s_term_history);

    s_is_processing_cmd = true;
    tab5_ui_textarea_set_text(s_ta, s_term_history);
    tab5_ui_textarea_set_cursor_pos(s_ta, TAB5_UI_CURSOR_LAST);
    s_is_processing_cmd = false;
}

static void app_init(void)
{
    tab5_system_log(2, "tab5_terminal", "Aplicativo Terminal interativo iniciado");
    tab5_ui_app_bar_set_title("Terminal");

    s_ta = tab5_ui_get_main_textarea();

    s_term_history[0] = '\0';
    strcat(s_term_history, "Tab5 OS Terminal Shell [v1.0.0]\n");
    strcat(s_term_history, "Digite 'help' para a lista de comandos disponiveis.\n\n");
    strcat(s_term_history, s_current_cwd);
    strcat(s_term_history, " $ ");
    s_prompt_min_index = strlen(s_term_history);

    if (s_ta != TAB5_UI_INVALID_OBJ) {
        s_is_processing_cmd = true;
        tab5_ui_textarea_set_text(s_ta, s_term_history);
        tab5_ui_textarea_set_cursor_pos(s_ta, TAB5_UI_CURSOR_LAST);
        s_is_processing_cmd = false;
    }
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

TAB5_APP_EXPORT void tab5_app_on_ui_event(tab5_ui_obj_t obj,
                                          uint32_t event_type,
                                          int32_t event_val)
{
    (void)event_val;
    if (obj != s_ta && s_ta != TAB5_UI_INVALID_OBJ) {
        return;
    }

    if (event_type == TAB5_UI_EVENT_CLICKED || event_type == TAB5_UI_EVENT_FOCUSED) {
        tab5_ui_keyboard_show(s_ta);
        int32_t cur = tab5_ui_textarea_get_cursor_pos(s_ta);
        if (cur < (int32_t)s_prompt_min_index) {
            tab5_ui_textarea_set_cursor_pos(s_ta, TAB5_UI_CURSOR_LAST);
        }
    } else if (event_type == TAB5_UI_EVENT_VALUE_CHANGED) {
        if (s_is_processing_cmd) {
            return;
        }
        const char *txt = tab5_ui_textarea_get_text(s_ta);
        if (txt == NULL) {
            return;
        }

        size_t cur_len = strlen(txt);
        /* Protege histórico e prompt contra exclusão indevida */
        if (cur_len < s_prompt_min_index || strncmp(txt, s_term_history, s_prompt_min_index) != 0) {
            s_is_processing_cmd = true;
            tab5_ui_textarea_set_text(s_ta, s_term_history);
            tab5_ui_textarea_set_cursor_pos(s_ta, TAB5_UI_CURSOR_LAST);
            s_is_processing_cmd = false;
            return;
        }

        /* Se o usuário digitou Enter ('\n') */
        if (cur_len > 0 && txt[cur_len - 1] == '\n') {
            execute_current_command();
        }
    }
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
        .on_ui_event = tab5_app_on_ui_event,
    };

    tab5_lifecycle_register(&cbs);
    app_init();
    return 0;
}
