#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_DISKS 3

// Estrutura para armazenar o estado atual do jogo
typedef struct {
    int pegs[3][MAX_DISKS];
    int h; // valor da heurística
    int g; // custo acumulado
    int f; // valor da função de avaliação (f = g + h)
    int moves; // quantidade de movimentos realizados
} State;

// Função para calcular a heurística de um estado
int heuristic(State *state) {
    int h = 0;
    for (int i = 0; i < MAX_DISKS; i++) {
        if (state->pegs[2][i] == 0) {
            h++;
        }
    }
    return h;
}

// Função para verificar se um estado é o estado final (todos os discos na terceira estaca)
int is_goal(State *state) {
    for (int i = 0; i < MAX_DISKS; i++) {
        if (state->pegs[2][i] == 0) {
            return 0;
        }
    }
    return 1;
}

// Função para copiar um estado
void copy_state(State *dest, State *src) {
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < MAX_DISKS; j++) {
            dest->pegs[i][j] = src->pegs[i][j];
        }
    }
    dest->h = src->h;
    dest->g = src->g;
    dest->f = src->f;
    dest->moves = src->moves; // Copia o valor de 'moves'
}


// Função para mover um disco de uma estaca para outra
void move_disk(State *state, int from, int to) {
    int disk = -1;
    for (int i = MAX_DISKS - 1; i >= 0; i--) {
        if (state->pegs[from][i] != 0) {
            disk = state->pegs[from][i];
            state->pegs[from][i] = 0;
            break;
        }
    }
    for (int i = 0; i < MAX_DISKS; i++) {
        if (state->pegs[to][i] == 0) {
            state->pegs[to][i] = disk;
            break;
        }
    }

    state->moves = state->moves + 1; // Atualiza o contador de movimentos
}


// Função para expandir um estado e gerar seus sucessores
State *expand_state(State *state, int *n_successors) {
    State *successors = malloc(sizeof(State) * 12);
    *n_successors = 0;

    for (int from = 0; from < 3; from++) {
        for (int to = 0; to < 3; to++) {
            if (from != to) {
                // Verifica se é possível mover um disco da estaca 'from' para a estaca 'to'
                int can_move = 1;
                int top_disk_from = -1;
                int top_disk_to = -1;

                for (int i = MAX_DISKS - 1; i >= 0; i--) {
                    if (state->pegs[from][i] != 0 && top_disk_from == -1) {
                        top_disk_from = state->pegs[from][i];
                    }
                    if (state->pegs[to][i] != 0 && top_disk_to == -1) {
                        top_disk_to = state->pegs[to][i];
                    }
                }

                if (top_disk_from == -1 || (top_disk_to != -1 && top_disk_to < top_disk_from)) {
                    can_move = 0;
                }

                if (can_move) {
                    State new_state;
                    copy_state(&new_state, state);
                    move_disk(&new_state, from, to);
                    new_state.g++;
                    new_state.h = heuristic(&new_state);
                    new_state.f = new_state.g + new_state.h;
                    new_state.moves = state->moves + 1; // Atualiza a quantidade de movimentos

                    copy_state(&successors[*n_successors], &new_state);
                    (*n_successors)++;
                }
            }
        }
    }

    return successors;
}




// Função para imprimir um estado
void print_state(State *state) {
    printf("h=%d g=%d f=%d moves=%d\n", state->h, state->g, state->f, state->moves);
    for (int j = MAX_DISKS - 1; j >= 0; j--) {
        for (int i = 0; i < 3; i++) {
            printf("%d ", state->pegs[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

int main() {
    // Estado inicial
    State initial_state = {
        .pegs = {{1, 2, 3}, {0, 0, 0}, {0, 0, 0}},
        .g = 0,
        .moves = 0
    };
    initial_state.h = heuristic(&initial_state);
    initial_state.f = initial_state.g + initial_state.h;

    // Lista aberta
    State open_list[100];
    int n_open = 1;
    copy_state(&open_list[0], &initial_state);

    // Lista fechada
    State closed_list[100];
    int n_closed = 0;

while (n_open > 0) {
    // Seleciona o estado com menor valor de f da lista aberta
    State current_state;
    copy_state(&current_state, &open_list[0]);
    for (int i = 0; i < n_open - 1; i++) {
        copy_state(&open_list[i], &open_list[i + 1]);
    }
    n_open--;

    // Verifica se o estado é o estado final
if (is_goal(&current_state)) {
    printf("Solução encontrada!\n");
    printf("Quantidade de movimentos: %d\n", current_state.g); // Mostra a quantidade de movimentos (valor de 'g')
    print_state(&current_state);
    break;
}


        // Adiciona o estado na lista fechada
        copy_state(&closed_list[n_closed], &current_state);
        n_closed++;

        // Expande o estado e gera seus sucessores
        int n_successors;
        State *successors = expand_state(&current_state, &n_successors);

        for (int i = 0; i < n_successors; i++) {
            State successor;
            copy_state(&successor, &successors[i]);

            // Verifica se o sucessor já está na lista fechada
            int in_closed = 0;
            for (int j = 0; j < n_closed; j++) {
                if (memcmp(&successor, &closed_list[j], sizeof(State)) == 0) {
                    in_closed = 1;
                    break;
                }
            }

            if (!in_closed) {
                // Verifica se o sucessor já está na lista aberta
                int in_open = 0;
                int open_index = -1;
                for (int j = 0; j < n_open; j++) {
                    if (memcmp(&successor, &open_list[j], sizeof(State)) == 0) {
                        in_open = 1;
                        open_index = j;
                        break;
                    }
                }

                if (in_open) {
                    // Se o sucessor já está na lista aberta, verifica se o novo caminho é melhor
                    if (successor.g < open_list[open_index].g) {
                        copy_state(&open_list[open_index], &successor);
                    }
                } else {
                    // Adiciona o sucessor na lista aberta
                    copy_state(&open_list[n_open], &successor);
                    n_open++;
                }
            }
        }

        free(successors);
    }

    return 0;
}
