#include <stdlib.h>
#include <time.h>
#include <ncurses.h>
#include <signal.h>
#include <string.h>

// compilation : gcc snake.c -o snake -lncurses
// 	rajouter un cadre et jouer dans le cadre
// 	rajouter la demande de rejouer
// 	reecrire le programme en utilisant ma bib Fen
// 	pb des couleurs aleatoires pas voulues
// 		normalement change de couleur en fonction de la nouriture mangée ...
// 		et le snake est censé demarrer avec la palette noir sur noir ...
//

#define VERSION	"Version 1.82"
#define AVEC_SON	1

#define CORP_SNAKE '*'
#define TETE_SNAKE 'O'

#define DU_MANGER '1'
#define CASE_VIDE ' '

#define FICHIER_DES_SCORES "score.hi"
#define SCORE	"score : "
#define AMANGER	"cases à manger : "
#define TITRE	"Snake "
#define PASSE_MURAILLE	"triche"

#define NB_MANGER_AU_DEPART	19
#define DELAY_MIN	50

#define VRAI 1
#define FAUX 0


// STRUCTURES POUR LE SNAKE
struct S_uneCellule {
  	int ligne;
  	int colonne;
  	struct S_uneCellule * suiv;
	};

struct S_unSnake {
  	struct S_uneCellule *teteSnake;
  	struct S_uneCellule *queueSnake;
	};

struct S_uneDirection {
  	int ligne;
  	int colonne;
	};


// Prototypages des fonctions
static void creerSnake(void);
static void ajouterEnTete(int,int,char *,char *);
static void supprimerQueue(void); 
static void initGrille(char **);
static void afficherGrille(WINDOW *);
static void genererDuManger(char **,int);
static void printFail(WINDOW *);
static void gererEvenement(int,char *,char *);
static void libererSnake(void); 
static void lectureScoreDansFichier(void);
static void ecrireScoreDansFichier(int,int);
static void finish(int);

// VARIABLES GLOBALES
char 	**grille = NULL,
	modePasseMuraille;// mode triche
int 	nbLignes = 0,
	nbColonnes = 0,
	meilleurScore = 0,meilleurAmanger = 0,score = 0,
	nbCellulesAjoutees = 0,
 	colorDuSnake; 
struct S_unSnake snake;
struct S_uneDirection direction = {0,1};
			       
			     
//Les fonctions
static void lectureScoreDansFichier() // lecture du score dans un fichier
{
FILE *file;

file = fopen(FICHIER_DES_SCORES, "r");
if(file != NULL) {
	fscanf(file,"Score:%d,NB Manger:%d;",&meilleurScore,&meilleurAmanger);
    	fclose(file); 
    	} 
}


// ecriture du score dans un fichier
static void ecrireScoreDansFichier(int n,int score) 
{
FILE *file;

file = fopen(FICHIER_DES_SCORES, "w");
if(file != NULL) {
    fprintf(file, "Score:%d,NB Manger:%d\n",score,n); 
    fclose(file); 
    } 
}


// Creation d'un snake de base
static void creerSnake(void) 
{
struct S_uneCellule	*teteSnake,*queueSnake,*milieuSnake;

teteSnake = (struct S_uneCellule *) malloc(sizeof(struct S_uneCellule));
queueSnake = (struct S_uneCellule *) malloc(sizeof(struct S_uneCellule));
milieuSnake = (struct S_uneCellule *) malloc(sizeof(struct S_uneCellule));

teteSnake->ligne = 1;
teteSnake->colonne = 4;
teteSnake->suiv = NULL;

milieuSnake->ligne = 1;
milieuSnake->colonne = 3;
milieuSnake->suiv = teteSnake;

queueSnake->ligne = 1;
queueSnake->colonne = 2;
queueSnake->suiv = milieuSnake;

snake.teteSnake = teteSnake;
snake.queueSnake = queueSnake;
}

// Redéfini la tête du snake aux coordonnées indiquées
// Gère les collisions avec les bords (sauf si mode triche) et le snake
// Vérifie si le snake mange quelque chose
static void ajouterEnTete(int ligne,int colonne,char *aMange,char *fail) 
{
struct S_uneCellule *nouvelleTete;
char	etat;

nouvelleTete = (struct S_uneCellule *) malloc(sizeof(struct S_uneCellule));

nouvelleTete->ligne = ligne;
nouvelleTete->colonne = colonne;
nouvelleTete->suiv = NULL;

snake.teteSnake->suiv = nouvelleTete;
grille[(snake.teteSnake->ligne)][(snake.teteSnake->colonne)] = CORP_SNAKE;
snake.teteSnake = snake.teteSnake->suiv;

// GESTION DES COLLISIONS avec la grille
if(modePasseMuraille) { 
	if(snake.teteSnake->ligne < 0) 
		snake.teteSnake->ligne = nbLignes - 1 - 3; 
	else if (snake.teteSnake->ligne > nbLignes - 1 - 3) 
		snake.teteSnake->ligne = 0; 
	else if (snake.teteSnake->colonne < 0) 
		snake.teteSnake->colonne = nbColonnes - 1; 
	else if (snake.teteSnake->colonne > nbColonnes - 1) 
		snake.teteSnake->colonne = 0; 
	}
else 
	if(snake.teteSnake->ligne < 0 
		|| snake.teteSnake->ligne > nbLignes - 1 - 3
		|| snake.teteSnake->colonne < 0 
		|| snake.teteSnake->colonne > nbColonnes - 1
		|| (grille[snake . teteSnake -> ligne][snake . teteSnake -> colonne] == CORP_SNAKE) 
		)
		*fail = VRAI;

if(!*fail) {
	//*aMange = (grille[snake->teteSnake->ligne][snake->teteSnake->colonne] == DU_MANGER) ? 1 : 0;
	etat = grille[snake.teteSnake -> ligne][snake.teteSnake -> colonne];
	if((etat >= DU_MANGER) && (etat < (DU_MANGER + 9)))	{
		*aMange = 1;
		nbCellulesAjoutees = etat - DU_MANGER; 
		score += etat - DU_MANGER + 1;
		}
	else
		*aMange = FAUX;

	grille[snake.teteSnake->ligne][snake.teteSnake->colonne] = TETE_SNAKE;
	// augmenter la taille ?  ici 
	}
}


// Supprime la queue du snake
static void supprimerQueue(void) 
{
struct S_uneCellule *auxi;

auxi = snake.queueSnake;
grille[snake.queueSnake->ligne][snake.queueSnake->colonne] = CASE_VIDE;
snake.queueSnake = snake.queueSnake->suiv;
free(auxi);
}

static void initGrille(char **g) 
{
int i,j;

for(i = 0;i < nbLignes - 3;i ++) 
	for(j = 0;j < nbColonnes;j ++) 
		g[i][j] = CASE_VIDE;
}

static void afficherGrille(WINDOW *win) 
{
int 	i,j;
char	g,c;
		  
for(i = 0; i < nbLignes - 3;i ++) 
	for (j = 0;j < nbColonnes;j ++) {
		g = grille[i][j];
		if((g >= DU_MANGER) && (g < (DU_MANGER + 9))) {
                	// Activer la couleur pour la nourriture
			c = g - DU_MANGER + 1; // de 1 a 9
                	wattron(win, COLOR_PAIR(c));
			wprintw(win,"%c",g);
                	wattroff(win, COLOR_PAIR(c));
			colorDuSnake = c + 10; // de 11 à 19
			}
		else if((g == CORP_SNAKE) || (g == TETE_SNAKE)) {
                	wattron(win, COLOR_PAIR(colorDuSnake));
			wprintw(win,"%c",g);
                	wattroff(win, COLOR_PAIR(colorDuSnake));
			}
		else 
			wprintw(win,"%c",g);
		}
}

static void gererEvenement(touche,fail,aMange) 
int touche; 
char *fail;
char *aMange; 
{
if(direction.ligne == 0) { // Pour ne pas 'aller en arrière'
	if(touche == KEY_UP){ 
		direction.ligne = -1;
		direction.colonne = 0; // Pour ne pas aller en diagonale
		}
	if(touche == KEY_DOWN) {
		direction.ligne = 1;
		direction.colonne = 0;
		}
	}

if(direction.colonne == 0) {
	if(touche == KEY_LEFT) {
		direction.colonne = -1;
		direction.ligne = 0;
		}
	if(touche == KEY_RIGHT) {
		direction.colonne = 1;
		direction.ligne = 0;
		}
	}

ajouterEnTete(	snake.teteSnake->ligne + direction.ligne,
		snake.teteSnake->colonne + direction.colonne, 
		aMange,
		fail);

if(!*aMange)
	supprimerQueue();
else {
	// OH
	// Ajouter des cellules supplémentaires en fonction de la valeur de la nourriture
	for(int i = 0; i < nbCellulesAjoutees; i++) {
		struct S_uneCellule *nouvelleCellule = (struct S_uneCellule *) malloc(sizeof(struct S_uneCellule));

		// Copier les coordonnées de la dernière queue
		nouvelleCellule->ligne = snake.queueSnake->ligne;
		nouvelleCellule->colonne = snake.queueSnake->colonne;
		nouvelleCellule->suiv = snake.queueSnake;

		// Mettre à jour la queue du serpent
		snake.queueSnake = nouvelleCellule;
		}
	}
}


// genere n case de bouffe à des coordonnées aléatoires
static void genererDuManger(char ** grille,int n) 
{
int 	ligne = 0,
	colonne = 0,
	i;
char 	done;

srand(time(NULL));
for(i = 0;i < n;i ++)	{
	done = FAUX;
	do {
		ligne = rand() % (nbLignes - 1 - 3);
		colonne = rand() % (nbColonnes - 1);
		if(grille[ligne][colonne] == CASE_VIDE) {
			grille[ligne][colonne] = DU_MANGER + (rand() % 9);
			done = VRAI;
			}
		} while (!done);
	}
}

static void printFail(WINDOW *win) 
{
/*
" _______  _______  _______  _______    _______           _______  _______ ";
"(  ____ \(  ___  )(       )(  ____ \  (  ___  )|\     /|(  ____ \(  ____ )";
"| (    \/| (   ) || () () || (    \/  | (   ) || )   ( || (    \/| (    )|";
"| |      | (___) || || || || (__      | |   | || |   | || (__    | (____)|";
"| | ____ |  ___  || |(_)| ||  __)     | |   | |( (   ) )|  __)   |     __)";
"| | \_  )| (   ) || |   | || (        | |   | | \ \_/ / | (      | (\ (   ";
"| (___) || )   ( || )   ( || (____/\  | (___) |  \   /  | (____/\| ) \ \__";
"(_______)|/     \||/     \|(_______/  (_______)   \_/   (_______/|/   \__/";
"                                                                          "; */
wmove(win, nbLignes / 2 - 5 / 2, nbColonnes / 2 - 74 / 2);
wprintw(win, " _______  _______  _______  _______    _______           _______  _______ \n");
wmove(win, 1 + nbLignes / 2 - 5 / 2, nbColonnes / 2 - 74 / 2);
wprintw(win, "(  ____ \\(  ___  )(       )(  ____ \\  (  ___  )|\\     /|(  ____ \\(  ____ )\n");
wmove(win, 2 + nbLignes / 2 - 5 / 2, nbColonnes / 2 - 74 / 2);
wprintw(win, "| (    \\/| (   ) || () () || (    \\/  | (   ) || )   ( || (    \\/| (    )|\n");
wmove(win, 3 + nbLignes / 2 - 5 / 2, nbColonnes / 2 - 74 / 2);
wprintw(win, "| |      | |   | || || || || (__      | |   | || |   | || (__    | (____)|\n");
wmove(win, 4 + nbLignes / 2 - 5 / 2, nbColonnes / 2 - 74 / 2);
wprintw(win, "| |      | |   | || |(_)| ||  __)     | |   | || |   | ||  __)   |     __)\n");
wmove(win, 5 + nbLignes / 2 - 5 / 2, nbColonnes / 2 - 74 / 2);
wprintw(win, "| |____/\\| (___) || |   | || (____/\\  | (___) || (___) || (____/\\| (\\ (   \n");
wmove(win, 6 + nbLignes / 2 - 5 / 2, nbColonnes / 2 - 74 / 2);
wprintw(win, "(_______/(_______)|/     \\|(_______/  (_______)(_______)(_______/|/   \\__/\n");
wrefresh(win);
}



// Fonction pour libérer la mémoire du serpent
static void libererSnake(void) 
{
struct S_uneCellule *current,*next;

current = snake.teteSnake;
// Libérer toutes les cellules du serpent
while(current != NULL) {
    next = current->suiv;
    free(current);
    current = next;
    }

snake.teteSnake = NULL;
snake.queueSnake = NULL;
}

static void finish(n)
int n;
{
int i;

// Libérer la mémoire de la grille et du serpent avant de quitter
for(i = 0;i < nbLignes - 3;i++) 
	free(grille[i]);
free(grille);

libererSnake();

endwin();
exit(n);
}

int main(int argc, char *argv[]) 
{
int 	i,
	touche, // touche pressee par le joueur
	nbCasesMangees, delay;
char	aMange,fail;
		

// INITIALISATIONS
modePasseMuraille = (argc > 1 && !strcmp(argv[1],PASSE_MURAILLE)) ? VRAI : FAUX;

creerSnake();
delay = nbCasesMangees = touche = 0;
aMange = VRAI;
fail = FAUX;

initscr();
keypad(stdscr, TRUE);
//OH
raw();  // Désactive le buffering du terminal
noecho();
cbreak();

/*
COLOR_BLACK	Noir
COLOR_RED	Rouge
COLOR_GREEN	Vert
COLOR_YELLOW	Jaune
COLOR_BLUE	Bleu
COLOR_MAGENTA	Magenta (violet)
COLOR_CYAN	Cyan (bleu clair)
COLOR_WHITE	Blanc */

if(has_colors()) {
	start_color();                  // Start color functionality
				
	// init_pair n°(entre 1 et COLOR_PAIRS - 1), couleur du texte, couleur du fond
	//init_pair(1, COLOR_BLACK,COLOR_RED); // par ex couleur noire sur fond rouge

	use_default_colors(); // utilise les couleurs de fond par defaut
	
	// couleurs de la nourriture
	init_pair(1, COLOR_RED, -1);  // Texte rouge sur le fond par défaut
	init_pair(2, COLOR_WHITE, COLOR_RED); 
	init_pair(3, COLOR_GREEN,-1);
	init_pair(4, COLOR_YELLOW,-1);
	init_pair(5, COLOR_BLUE,-1);
	init_pair(6, COLOR_CYAN,-1);
	init_pair(7, COLOR_MAGENTA,-1);
	init_pair(8, COLOR_BLUE,-1);
	init_pair(9, COLOR_BLACK,-1);

	// couleurs du snake
	colorDuSnake = 11;
	init_pair(11, COLOR_BLACK,COLOR_BLACK);
	init_pair(12, COLOR_RED,COLOR_RED);
	init_pair(13, COLOR_WHITE,COLOR_WHITE);
	init_pair(14, COLOR_GREEN,COLOR_GREEN);
	init_pair(15, COLOR_YELLOW,COLOR_YELLOW);
	init_pair(16, COLOR_BLUE,COLOR_BLUE);
	init_pair(17, COLOR_CYAN,COLOR_CYAN);
	init_pair(18, COLOR_MAGENTA,COLOR_MAGENTA);
	init_pair(19, COLOR_BLUE,COLOR_BLUE);

	// couleur des fenetres score et game
	init_pair(10, COLOR_BLACK,-1);
	}

//les signaux ...
(void) signal(SIGINT, finish);           /* arrange interrupts to terminate */
signal(SIGABRT,finish);
signal(SIGQUIT,finish);
signal(SIGKILL,finish);
signal(SIGTERM,finish);
signal(SIGCONT,finish);

// recuperation des records
lectureScoreDansFichier();

// Récupérer les dimensions de la fenêtre par défaut
getmaxyx(stdscr,nbLignes,nbColonnes);

WINDOW *game_win = newwin(nbLignes - 3, nbColonnes,3,0);  // Fenêtre de jeu plus petite
WINDOW *score_win = newwin(3, nbColonnes,0,0);  // Fenêtre pour le score en haut

if(has_colors()) {
	wbkgd(game_win,COLOR_PAIR(10));
	wbkgd(score_win,COLOR_PAIR(10));
	}

//@mvwprintw(score_win,1, 2,"meilleur score:%d",meilleurScore);
mvwprintw(score_win,1, 2,"Record : %d",meilleurScore);
mvwprintw(score_win,1, nbColonnes / 4,"%s%s",TITRE,VERSION);
mvwprintw(score_win,1, nbColonnes / 2,"%s%d",SCORE,score);
mvwprintw(score_win,1, 3 * nbColonnes / 4,"%s",AMANGER);
wrefresh(score_win);

keypad(game_win, TRUE);

// Initialiser la grille
grille = (char **) malloc((nbLignes - 3) * sizeof(char *));
for(i = 0; i < nbLignes - 3; i++) 
     	grille[i] = (char *) malloc(nbColonnes * sizeof(char));

initGrille(grille);
genererDuManger(grille,NB_MANGER_AU_DEPART);

box(game_win,0,0); // marche pas ...
wrefresh(game_win);

// BOUCLE DE JEU
do	{
	if(aMange)	{
		genererDuManger(grille,1);

		box(score_win,0,0);
		mvwprintw(score_win,1,strlen(SCORE) + nbColonnes / 2,"%d",score);
		mvwprintw(score_win,1,nbColonnes - strlen(AMANGER),"%d",nbCasesMangees);
		wrefresh(score_win);

		nbCasesMangees ++;
#ifdef AVEC_SON
		beep();
#endif

		delay = 101 - nbCasesMangees;
		delay = (delay < DELAY_MIN) ? DELAY_MIN : delay;
		wtimeout(game_win,delay); // On raffraichi toutes les 60 ms au max
		}

	afficherGrille(game_win);
	touche = wgetch(game_win);
	gererEvenement(touche,&fail,&aMange);
	werase(game_win);
	} while(!fail);

// Le joueur a fail
//OH
if(score > meilleurScore && !modePasseMuraille)
	ecrireScoreDansFichier(nbCasesMangees - 1,score); // Write the score to the file

wtimeout(game_win,5000);
werase(game_win);
printFail(game_win);
wgetch(game_win);

finish(0);
}
