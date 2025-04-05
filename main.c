//all

//inclue les librairie utiliser
#include <ncurses.h>
#include <stdlib.h>
#include <time.h>
//ici on defini la taille du plateau de jeux et le nombre de case 3*3=9
#define TAILLE 3
#define MAX_MOVES 9
//on defini l'utilisation de x y pour les position
typedef struct {
    int x;
    int y;
} Position;
//les variables
char plateau[TAILLE][TAILLE];
char joueurActuel = 'X';
int jeuTermine = 0;
Position historiqueMouvements[MAX_MOVES];
int nombreMouvements = 0;
//les voids sont des petit code qu'on utilise a plusieur endroit grace a cela on peut les appeler autant de fois qu'on le souhaite
void initialiserPlateau();
void afficherPlateau(WINDOW *win);
int verifierGagnant();
int verifierMatchNul();
void mouvementJoueur(WINDOW *win);
Position getPositionCurseur(WINDOW *win);
void rejouerPartie(WINDOW *win);
void mouvementIA();
void mouvementIARandom();
void afficherMenu();

int main() {
    //intialise pour ncurses
    initscr();
    cbreak();
    noecho();
    //touche special fleche pour les deplacements
    keypad(stdscr, TRUE);
    //sera utilise pour generer de l'aleatoire
    srand(time(NULL));
    //appele du void
    afficherMenu();

    endwin();
    return 0;
}
//affichage du menu
void afficherMenu() {
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);
    char *menu_items[] = {"Joueur vs Joueur", "Joueur vs IA", "IA vs IA", "Replay", "Quitter"};
    int num_items = 5;
    int selected_item = 0;
//affiche le menu jusqu'a ce que le joueur quite
    while (1) {
        clear();
        //titre
        mvprintw(max_y / 4, (max_x - 10) / 2, "TIC-TAC-TOE");
        for (int i = 0; i < num_items; i++) {
            if (i == selected_item) attron(A_REVERSE);
            mvprintw(max_y / 4 + 2 + i, (max_x - 10) / 2, "%s", menu_items[i]);
            attroff(A_REVERSE);
        }
        //int pour recupere les entrees 
        int ch = getch();
        //traite le get pour reagir celon les touches frapper
        switch (ch) {
            case KEY_UP: selected_item = (selected_item - 1 + num_items) % num_items; break;
            case KEY_DOWN: selected_item = (selected_item + 1) % num_items; break;
            case 10:
            // pour les trois premier modes on affiche le plateau
                if (selected_item == 0 || selected_item == 1 || selected_item == 2) {
                    WINDOW *plateauWin = newwin(TAILLE * 3 + 3, TAILLE * 5 + 3, 5, 10);
                    box(plateauWin, 0, 0);
                    refresh();
                    initialiserPlateau();
                    afficherPlateau(plateauWin);
                    jeuTermine = 0;
                    joueurActuel = 'X';
                    nombreMouvements = 0;
                    //boucle de jeux
                    while (!jeuTermine) {
                        //mode 2 tour IA
                        if ((selected_item == 1 && joueurActuel == 'O')) {
                            mouvementIARandom();
                            //mode 3 IA tour IA
                        } else if (selected_item == 2) {
                            mouvementIARandom();
                            //tour joueur
                        } else {
                            Position pos = getPositionCurseur(plateauWin);
                            plateau[pos.y][pos.x] = joueurActuel;
                            historiqueMouvements[nombreMouvements].x = pos.x;
                            historiqueMouvements[nombreMouvements].y = pos.y;
                            nombreMouvements++;
                        }
                        //verification de fin d'une partie ou non
                        afficherPlateau(plateauWin);
                        if (verifierGagnant() || verifierMatchNul()) {
                            jeuTermine = 1;
                            mvwprintw(plateauWin, TAILLE * 3 + 1, 1, verifierGagnant() ? "Joueur %c a gagné" : "Match nul!", joueurActuel);
                            //si parti pas fini changelent de joueur
                        } else {
                            joueurActuel = (joueurActuel == 'X') ? 'O' : 'X';
                            mvwprintw(plateauWin, TAILLE * 3 + 1, 1, "Joeur : %c ", joueurActuel);
                        }
                        wrefresh(plateauWin);
                    }
                    getch();
                    delwin(plateauWin);
                    clear();
                    refresh();
                    afficherMenu();
                    return;
                    //mode replay
                } else if (selected_item == 3) {
                    WINDOW *replayWin = newwin(TAILLE * 3 + 3, TAILLE * 5 + 3, 5, 10);
                    box(replayWin, 0, 0);
                    refresh();
                    rejouerPartie(replayWin);
                    getch();
                    delwin(replayWin);
                    clear();
                    refresh();
                    afficherMenu();
                    return;
                } else {
                    //pour quitter
                    return;
                }
                break;
        }
        refresh();
    }
}
//si la grille est deja la simplement la vider
void initialiserPlateau() {
    for (int i = 0; i < TAILLE; i++) {
        for (int j = 0; j < TAILLE; j++) {
            plateau[i][j] = ' ';
        }
    }
}
//afficher la grille
void afficherPlateau(WINDOW *win) {
    for (int i = 0; i < TAILLE; i++) {
        for (int j = 0; j < TAILLE; j++) {
            mvwprintw(win, i * 3 + 2, j * 5 + 3, "%c", plateau[i][j]);
            if (j < TAILLE - 1) mvwvline(win, i * 3 + 1, j * 5 + 5, ACS_VLINE, 3);
            if (i < TAILLE - 1) mvwhline(win, i * 3 + 3, j * 5 + 1, ACS_HLINE, 5);
            if (i < TAILLE - 1 && j < TAILLE - 1) mvwaddch(win, i * 3 + 3, j * 5 + 5, ACS_PLUS);
        }
    }
    wrefresh(win);
}
//verifier si 3 symboles indentique sont aligner
int verifierGagnant() {
    for (int i = 0; i < TAILLE; i++) {
        if (plateau[i][0] == plateau[i][1] && plateau[i][1] == plateau[i][2] && plateau[i][0] != ' ') return 1;
        if (plateau[0][i] == plateau[1][i] && plateau[1][i] == plateau[2][i] && plateau[0][i] != ' ') return 1;
    }
    if ((plateau[0][0] == plateau[1][1] && plateau[1][1] == plateau[2][2] && plateau[0][0] != ' ') ||
        (plateau[0][2] == plateau[1][1] && plateau[1][1] == plateau[2][0] && plateau[0][2] != ' ')) {
        return 1;
    }
    return 0;
}
//si toute les case sont remplie
int verifierMatchNul() {
    for (int i = 0; i < TAILLE; i++) {
        for (int j = 0; j < TAILLE; j++) {
            if (plateau[i][j] == ' ') return 0;
        }
    }
    return 1;
}

//mouvement des robots aleatoires
void mouvementIARandom() {
    int x, y;
    do {
        x = rand() % TAILLE;
        y = rand() % TAILLE;
    } while (plateau[y][x] != ' ');

    plateau[y][x] = joueurActuel;
    historiqueMouvements[nombreMouvements].x = x;
    historiqueMouvements[nombreMouvements].y = y;
    nombreMouvements++;
}
// pour afoir la position selectionner
Position getPositionCurseur(WINDOW *win) {
    Position pos = {0, 0};
    int ch;
    while (1) {
        ch = getch();
        switch (ch) {
            case KEY_UP: pos.y = (pos.y - 1 + TAILLE) % TAILLE; break;
            case KEY_DOWN: pos.y = (pos.y + 1) % TAILLE; break;
            case KEY_LEFT: pos.x = (pos.x - 1 + TAILLE) % TAILLE; break;
            case KEY_RIGHT: pos.x = (pos.x + 1) % TAILLE; break;
            case 10: if (plateau[pos.y][pos.x] == ' ') return pos;
        }
        wmove(win, pos.y * 3 + 2, pos.x * 5 + 3);
        wrefresh(win);
    }
}
void rejouerPartie(WINDOW *win) {
    initialiserPlateau();
    afficherPlateau(win);
    joueurActuel = 'X';

    for (int i = 0; i < nombreMouvements; i++) {
        plateau[historiqueMouvements[i].y][historiqueMouvements[i].x] = joueurActuel;
        afficherPlateau(win);
        wrefresh(win);
        napms(500); // Pause de 500 millisecondes entre les mouvements
        joueurActuel = (joueurActuel == 'X') ? 'O' : 'X';
    }
    wrefresh(win);
}
