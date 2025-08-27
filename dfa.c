#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_STATES 128
#define MAX_ALPHA 128
#define MAX_LINE 1024
#define MAX_NAME 128

static char states[MAX_STATES][MAX_NAME];
static int n_states = 0;
static char alphabet[MAX_ALPHA]; // símbolos de 1 char
static int n_alpha = 0;
static int start_idx = -1;
static int accept_mask[MAX_STATES];
static int trans[MAX_STATES][MAX_ALPHA]; // -1 si no definida

static int is_comment_or_empty(const char *s){
    while (*s && isspace((unsigned char)*s)) s++;
    return (*s == '\0' || *s == '#');
}

static int trim(char *s){
    int len = (int)strlen(s);
    while (len>0 && (s[len-1]=='\n' || s[len-1]=='\r')) s[--len] = '\0';
    int i=0; while (isspace((unsigned char)s[i])) i++;
    if(i>0) memmove(s, s+i, len - i + 1);
    len = (int)strlen(s);
    while (len>0 && isspace((unsigned char)s[len-1])) s[--len] = '\0';
    return len;
}

static int find_state(const char *name){
    for (int i=0;i<n_states;i++) if (strcmp(states[i], name)==0) return i;
    return -1;
}

static int add_state(const char *name){
    if (n_states >= MAX_STATES) { fprintf(stderr, "Demasiados estados\n"); exit(1);}    
    strncpy(states[n_states], name, MAX_NAME-1);
    states[n_states][MAX_NAME-1] = '\0';
    return n_states++;
}

static int find_symbol(char c){
    for (int i=0;i<n_alpha;i++) if (alphabet[i]==c) return i;
    return -1;
}

static void parse_csv_states(char *vals){
    char *tok = strtok(vals, ",");
    while (tok){
        trim(tok);
        if (*tok){
            if (find_state(tok) == -1) add_state(tok);
        }
        tok = strtok(NULL, ",");
    }
}

static void parse_csv_alphabet(char *vals){
    char *tok = strtok(vals, ",");
    while (tok){
        trim(tok);
        if (*tok){
            if (strlen(tok) != 1){ fprintf(stderr, "Símbolo '%s' debe ser de 1 carácter\n", tok); exit(1);}            
            if (n_alpha >= MAX_ALPHA){ fprintf(stderr, "Alfabeto demasiado grande\n"); exit(1);}            
            if (find_symbol(tok[0]) == -1) alphabet[n_alpha++] = tok[0];
        }
        tok = strtok(NULL, ",");
    }
}

static void init_trans(){
    for (int i=0;i<MAX_STATES;i++) for (int j=0;j<MAX_ALPHA;j++) trans[i][j] = -1;
    for (int i=0;i<MAX_STATES;i++) accept_mask[i] = 0;
}

static void parse_conf(const char *path){
    FILE *f = fopen(path, "r");
    if (!f){ perror("Conf.txt"); exit(1);}    
    init_trans();

    char line[MAX_LINE];
    int in_transitions = 0;

    while (fgets(line, sizeof(line), f)){
        trim(line);
        if (is_comment_or_empty(line)) continue;

        if (!in_transitions){
            if (strncasecmp(line, "states:", 7) == 0){
                char *vals = line + 7; while (*vals==':'||isspace((unsigned char)*vals)) vals++;
                parse_csv_states(vals);
            } else if (strncasecmp(line, "alphabet:", 9) == 0){
                char *vals = line + 9; while (*vals==':'||isspace((unsigned char)*vals)) vals++;
                parse_csv_alphabet(vals);
            } else if (strncasecmp(line, "start:", 6) == 0){
                char *val = line + 6; while (*val==':'||isspace((unsigned char)*val)) val++;
                trim(val);
                int idx = find_state(val);
                if (idx == -1){ fprintf(stderr, "Estado start '%s' no está en states\n", val); exit(1);}                
                start_idx = idx;
            } else if (strncasecmp(line, "accept:", 7) == 0){
                char buf[MAX_LINE]; strncpy(buf, line + 7, sizeof(buf)-1); buf[sizeof(buf)-1]='\0';
                char *tok = strtok(buf, ",");
                while (tok){
                    trim(tok);
                    if (*tok){
                        int idx = find_state(tok);
                        if (idx == -1){ fprintf(stderr, "Estado de accept '%s' no está en states\n", tok); exit(1);}                        
                        accept_mask[idx] = 1;
                    }
                    tok = strtok(NULL, ",");
                }
            } else if (strncasecmp(line, "transitions:", 11) == 0){
                in_transitions = 1;
            } else {
                fprintf(stderr, "Línea desconocida: %s\n", line);
                exit(1);
            }
        } else {
            // formato: estado,simbolo->destino
            char *arrow = strstr(line, "->");
            if (!arrow){ fprintf(stderr, "Transición inválida: %s\n", line); exit(1);}            
            *arrow = '\0';
            char *right = arrow + 2;
            trim(right);

            char left[MAX_LINE]; strncpy(left, line, sizeof(left)-1); left[sizeof(left)-1]='\0';
            char *comma = strchr(left, ',');
            if (!comma){ fprintf(stderr, "Transición inválida (falta coma): %s\n", line); exit(1);}            
            *comma = '\0';
            char *st = left; trim(st);
            char *symstr = comma + 1; trim(symstr);
            if (strlen(symstr) != 1){ fprintf(stderr, "Símbolo '%s' debe ser de 1 carácter\n", symstr); exit(1);}            
            char sym = symstr[0];

            int qi = find_state(st);
            int si = find_symbol(sym);
            int qj = find_state(right);
            if (qi==-1 || si==-1 || qj==-1){
                fprintf(stderr, "Transición con estado/símbolo desconocido: %s,%c->%s\n", st, sym, right);
                exit(1);
            }
            trans[qi][si] = qj;
        }
    }

    fclose(f);
    if (n_states==0 || n_alpha==0 || start_idx<0){
        fprintf(stderr, "Configuración incompleta (states/alphabet/start)\n");
        exit(1);
    }
}

static int simulate_line(const char *s){
    int q = start_idx;
    for (const char *p=s; *p; ++p){
        char c = *p;
        if (c=='\n' || c=='\r') break;
        if (isspace((unsigned char)c)) continue; // opcional: ignorar espacios
        int si = find_symbol(c);
        if (si==-1) return 0; // símbolo fuera del alfabeto
        int qn = trans[q][si];
        if (qn < 0) return 0; // transición no definida
        q = qn;
    }
    return accept_mask[q];
}

int main(int argc, char **argv){
    const char *conf = (argc==3) ? argv[1] : "Conf.txt";
    const char *words = (argc==3) ? argv[2] : "Cadenas.txt";
    parse_conf(conf);

    FILE *f = fopen(words, "r");
    if (!f){ perror("Cadenas.txt"); return 1; }

    char line[MAX_LINE];
    while (fgets(line, sizeof(line), f)){
        char tmp[MAX_LINE];
        strncpy(tmp, line, sizeof(tmp)-1); tmp[sizeof(tmp)-1]='\0';
        trim(tmp);
        if (is_comment_or_empty(tmp)) continue;
        int ok = simulate_line(tmp);
        printf("%s: %s\n", tmp, ok?"ACEPTADA":"RECHAZADA");
    }

    fclose(f);
    return 0;
}
