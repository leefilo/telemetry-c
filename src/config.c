#include "config.h"
#include <yaml.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


// Sensors are hardcoded -> brute check key
static void set_field(sensor_cfg_t *s, const char *key, const char *text) {
    if (strcmp(key, "id") == 0) {
        strncpy(s->id, text, sizeof s->id - 1);
        s->id[sizeof s->id - 1] = '\0';
    } else if (strcmp(key, "unit") == 0) {
        strncpy(s->unit, text, sizeof s->unit - 1);
        s->unit[sizeof s->unit - 1] = '\0';
    } else if (strcmp(key, "min") == 0) {
        char *e = NULL;
        double v = strtod(text, &e);
        if (e && *e == '\0') s->min = v;
        else fprintf(stderr, "bad min: %s\n", text);
    } else if (strcmp(key, "max") == 0) {
        char *e = NULL;
        double v = strtod(text, &e);
        if (e && *e == '\0') s->max = v;
        else fprintf(stderr, "bad max: %s\n", text);
    } else if (strcmp(key, "period_ms") == 0) {
        char *e = NULL;
        unsigned long v = strtoul(text, &e, 10);
        if (e && *e == '\0') s->period_ms = (unsigned)v;
        else fprintf(stderr, "bad period_ms: %s\n", text);
    } else {
        // unknown keys are ignored for now
    }
}


// Using EXP (expect) variables as indicators to keep track while parsing
// EXP_KEY means the next scalar we are expecting is a key
// EXP_VALUE means the next scalar we are expecting is a value
typedef enum { EXP_NONE, EXP_KEY, EXP_VALUE } expect_t;


// Current YAML file structure holds different sensors with the same values, so flags can check for a new sensor
static int parse_one_sensor(yaml_parser_t *parser, sensor_cfg_t *out) {
    expect_t expect = EXP_NONE;
    char curr_key[32] = {0};
    yaml_token_t t;

    memset(out, 0, sizeof *out);

    for (;;) {
        if (!yaml_parser_scan(parser, &t)) {
            fprintf(stderr, "YAML scan failed inside sensor mapping\n");
            return 1;
        }

        if (t.type == YAML_BLOCK_END_TOKEN) {  
            yaml_token_delete(&t);
            break;
        }

        switch (t.type) {

            // While parsing a sensor, use indicators when expecting a key or a value
            case YAML_KEY_TOKEN:   
                expect = EXP_KEY;   
                break;
            case YAML_VALUE_TOKEN: 
                expect = EXP_VALUE; 
                break;

            case YAML_SCALAR_TOKEN: {
                char text[64] = {0};
                size_t n = t.data.scalar.length;
                if (n >= sizeof text) n = sizeof text - 1;
                memcpy(text, t.data.scalar.value, n);
                text[n] = '\0';

                if (expect == EXP_KEY) {
                    strncpy(curr_key, text, sizeof curr_key - 1);
                    curr_key[sizeof curr_key - 1] = '\0';
                    expect = EXP_NONE;
                } else if (expect == EXP_VALUE) {
                    set_field(out, curr_key, text);
                    expect = EXP_NONE;
                }
                break;
            }

            default: 
            /* ignore */ 
            break;
        }

        yaml_token_delete(&t);
    }

    return 0;
}


// Parse and open the YAML file
int load_config(const char *path, car_sensors_t *out) {

    if (!out) return 1;
    memset(out, 0, sizeof *out);

    // Open file to read (path defined in main)
    FILE *f = fopen(path, "rb");           
    if (!f) {                              
        perror(path);                      
        return 1;
    }

    // Start parser
    yaml_parser_t parser;
    if (!yaml_parser_initialize(&parser)) {
        fprintf(stderr, "yaml parser init failed\n");
        fclose(f);
        return 1;
    }
    yaml_parser_set_input_file(&parser, f);

    yaml_token_t tok;
    yaml_token_type_t prev = YAML_NO_TOKEN;  // Remember previous token type
    int rc = 0;


    while (yaml_parser_scan(&parser, &tok)) {
        if (tok.type == YAML_STREAM_END_TOKEN) {
            yaml_token_delete(&tok);
            break;
        }

        // New sensor starts when: BLOCK_ENTRY then BLOCK_MAPPING_START
        if (prev == YAML_BLOCK_ENTRY_TOKEN && tok.type == YAML_BLOCK_MAPPING_START_TOKEN) {
            yaml_token_delete(&tok);              // Throw away the MAPPING_START token

            sensor_cfg_t cur;
            if (parse_one_sensor(&parser, &cur) != 0) { rc = 1; break; }

            if (out->count < 4) out->s[out->count++] = cur;     // Only storing 4 sensors for now
            else fprintf(stderr, "Too many sensors; expected 4\n");

            prev = YAML_NO_TOKEN;                 // Reset
            continue;
        }

            prev = tok.type;                          // Remember last token
            yaml_token_delete(&tok);                  // Delete token just used
    }


    // Cleanup
    yaml_parser_delete(&parser);
    fclose(f);

    return 0;

}