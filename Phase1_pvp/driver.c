//WIP
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <string.h>

#define GRID_SIZE 10
#define MAX_NAME_LENGTH 50
#define TOTALNUMBEROFSHIPS 4


//structures
typedef struct {
    char name[20];          // Ship name
    int size;               // Ship size
    char id;                // Ship identifier
    int occupiedCells[5][3];
    // (row, col, hit_status); 3rd column tracks hit status: 0 for not hit, 1 for hit max 5 cells for largest ship.              
} Ship;

typedef struct player {
    char name[MAX_NAME_LENGTH];
    int turn;                               //1 its their turn, 0 its not their turn.
    char board[GRID_SIZE][GRID_SIZE];       //contains placement of ships across the 10x10 board //tentative could be removed.
    char hits[GRID_SIZE][GRID_SIZE];        // grid with ~ for water, o for miss, * for hit.This is for hits and misses done by the opponent on this player's board
    int numOfShipsSunken;                   //when initializing players, set to zero. Useful for victory checking.Ships sunken by the opponent of this player's ships.
    Ship ships[TOTALNUMBEROFSHIPS];         //array of their ships. to be initialized.
    int numOfRadars;                        //allowed 3 per game.to be initialized
    int numOfSmokeScreensPerformed; 
    int numOfArtillery;                     //only ever 1 or 0
    int numOfTorpedo;                       //only ever 1 or 0
    char obscuredArea[GRID_SIZE][GRID_SIZE];
} Player;


//useful variables                       
char affectedArea[GRID_SIZE][GRID_SIZE];   // temporary grid for affected cells per each move.clears after every turn.

//All function prototypes
char set_game_difficulty();
void displayAvailableMoves();
void initialize_player(Player* player);
void initialize_board(char board[GRID_SIZE][GRID_SIZE]);
void playerswitch(Player *attacker, Player *defender);
void display_opponent_grid(char board[GRID_SIZE][GRID_SIZE], char game_difficulty);
void displayBoard(Player *player);
int column_to_index(char column);
int checkShipOverlap(Player *player, Ship *ship, int startRow, int startCol, char orientation);
void placeShipOnBoard(Player *player, Ship *ship, int startRow, int startCol, char orientation);
void placeShips(Player *player);
int is_fire(char* moveType);
int is_artillery(char* moveType);
int is_torpedo(char* moveType);
int is_radar(char* moveType);
int is_smoke(char* moveType);
int is_equal(char* str1, char* str2);
void FireMove(Player* attacker, Player* defender, int x, int y, char game_difficulty);
void ArtilleryMove(Player* attacker, Player* defender, int x, int y, char game_difficulty);
void TorpedoMove(Player* attacker, Player* defender, int x, int y, char game_difficulty);
void RadarMove(Player *attacker, Player *defender, int x, int y);
void SmokeMove(Player *attacker, int x, int y);
void selectMove(Player *attacker, Player *defender, char game_difficulty);
void HitOrMiss(Player *attacker, Player *defender, int x, int y, char movetype, char orientation, char game_difficulty);
void markAffectedArea(int x, int y, char moveType, char orientation);
int isShipSunk(Ship *ship);
void HitOrMissMessageDisplay(int movesuccess);
void startGame(Player *currentPlayer, Player *opponent, char game_difficulty);
void stringcopy(char* dest,char* src);
void to_lowercase(char* src, char* dest);


char set_game_difficulty() {
    char difficulty;
    
    while (1) {
        printf("Enter game difficulty (E for Easy, H for Hard): ");
        scanf(" %c", &difficulty); // Leading space in format string skips whitespace
        difficulty = toupper(difficulty);

        if (difficulty == 'E' || difficulty == 'H') {
            break; // Valid input, exit loop
        } else {
            printf("Invalid input. Please enter 'E' or 'H'.\n");
            while (getchar() != '\n'); // Clear remaining characters from the input buffer
        }
    }
    return difficulty;
}

void main(){
    srand(time(0));
    char EXIT;
    do
    {
        Player player1, player2;
        char game_difficulty = set_game_difficulty();
        printf("game difficulty set to : %c\n",game_difficulty);

        printf("Player 1, Enter your name: ");
        scanf("%s", player1.name);
        printf("Player 2, Enter your name: ");
        scanf("%s", player2.name);

        initialize_player(&player1);
        initialize_player(&player2);

    
        printf("Randomly choosing the starting player\n");
        if (rand() % 2 == 0) {
            player1.turn = 1;
            player2.turn = 0;
            printf("%s goes first!\n", player1.name);
        } else {
            player1.turn = 0;
            player2.turn = 1;
            printf("%s goes first!\n", player2.name);
        }
        printf("Placing %s's ships\n", player1.name);
        placeShips(&player1);
        printf("Placing %s's ships\n", player2.name);
        placeShips(&player2);
    
        startGame(&player1, &player2, game_difficulty);

        //prompt if players would like to play again
        printf("Would you like to play again ? ^-^ (Y/N)");
        scanf("%c", &EXIT);

    } while (EXIT=='Y' || EXIT=='y');
    printf("Thanks for playing ^-^ ");   
}
void startGame(Player *player1, Player *player2,char game_difficulty ) {
    while (player1->numOfShipsSunken < TOTALNUMBEROFSHIPS && player2->numOfShipsSunken < TOTALNUMBEROFSHIPS) { //win condition
        // Determine current attacker and defender based on the turn parameter in player struct
        Player *attacker = player1->turn == 1 ? player1 : player2;
        Player *defender = player1->turn == 1 ? player2 : player1;
   
        selectMove(attacker, defender, game_difficulty); //this handles switching turns accordingly after a valid move
    }

    //display the winner
    if (player1->numOfShipsSunken == TOTALNUMBEROFSHIPS) {
        printf("%s wins!\n", player2->name);
    } else {
        printf("%s wins!\n", player1->name);
    }
    //to find out where each player hid their ships in the end
    displayBoard(player1); 
    displayBoard(player2);
}

void clear_screen() { //depedming on system change the command
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}
void displayAvailableMoves() {
    printf("Available Moves:\n");
    printf("1. Fire: Attempts to hit an opponent's ship at a specified coordinate.\n");
    printf("   Command: Fire [coordinate] (e.g., Fire B3)\n");
    printf("2. Radar Sweep: Reveals if there are enemy ships in a 2x2 area.\n");
    printf("   Command: Radar [top-left coordinate] (e.g. Radar B3)\n");
    printf("   Note: Limited to 3 uses per player.\n");
    printf("3. Smoke Screen: Obscures a 2x2 area of the grid from radar sweeps.\n");
    printf("   Command: Smoke [top-left coordinate] (e.g. Smoke B3)\n");
    printf("   Note: Allowed one screen per sunken ship.\n");
    printf("4. Artillery: Targets a 2x2 area, functioning similarly to Fire.\n");
    printf("   Command: Artillery [top-left coordinate] (e.g. Artillery B3)\n");
    printf("   Note: Unlocked next turn if an opponent's ship is sunk.\n");
    printf("5. Torpedo: Targets an entire row or column.\n");
    printf("   Command: Torpedo [row/column] (e.g. Torpedo B)\n");
    printf("   Note: Unlocked next turn if a third ship is sunk.\n");
}

void initialize_player(Player* player){
    player->turn = 0; // Set turn to 0
    player->numOfShipsSunken = 0; // Initialize sunk ships count
    player->numOfArtillery=0;
    player->numOfRadars=3;
    player->numOfSmokeScreensPerformed=0;
    player->numOfTorpedo=0;
    initialize_board(player->board);        // Initialize player's board
    initialize_board(player->hits);         // Initialize hits board
    initialize_board(player->obscuredArea); // Initialize obscuredArea board

    //filling the ship array
    // Carrier
    stringcopy(player->ships[0].name, "Carrier");
    player->ships[0].size = 5;
    player->ships[0].id = 'C';
    for (int i = 0; i < 5; i++) {
        player->ships[0].occupiedCells[i][0] = 0; // Row
        player->ships[0].occupiedCells[i][1] = 0; // Column
        player->ships[0].occupiedCells[i][2] = 0; // Hit status
    }

    // Battleship
    stringcopy(player->ships[1].name, "Battleship");
    player->ships[1].size = 4;
    player->ships[1].id = 'B';
    for (int i = 0; i < 4; i++) {
        player->ships[1].occupiedCells[i][0] = 0; // Row
        player->ships[1].occupiedCells[i][1] = 0; // Column
        player->ships[1].occupiedCells[i][2] = 0; // Hit status
    }

    // Destroyer
    stringcopy(player->ships[2].name, "Destroyer");
    player->ships[2].size = 3;
    player->ships[2].id = 'D';
    for (int i = 0; i < 3; i++) {
        player->ships[2].occupiedCells[i][0] = 0;
        player->ships[2].occupiedCells[i][1] = 0;
        player->ships[2].occupiedCells[i][2] = 0;
    }

    // Submarine
    stringcopy(player->ships[3].name, "Submarine");
    player->ships[3].size = 2;
    player->ships[3].id = 'S';
    for (int i = 0; i < 2; i++) {
        player->ships[3].occupiedCells[i][0] = 0;
        player->ships[3].occupiedCells[i][1] = 0;
        player->ships[3].occupiedCells[i][2] = 0;
    }

}

void initialize_board(char board[GRID_SIZE][GRID_SIZE]) {
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            board[i][j] = '~';
        }
    }
}

void playerswitch(Player *attacker, Player *defender) {
    attacker->turn=1-attacker->turn;
    defender->turn=1-defender->turn;
}


void display_opponent_grid(char board[GRID_SIZE][GRID_SIZE], char game_difficulty) { 
    
    //print column indices
    printf("   ");
    for (int j = 0; j < GRID_SIZE; j++) {
        printf("%c ", 'A' + j);
    }
    printf("\n");

    for (int i = 0; i < GRID_SIZE; i++) {
        // Print row number (1-11)
        printf("%2d ", i + 1);  

        for (int j = 0; j < GRID_SIZE; j++) {
            if (game_difficulty == 'E') {
                // Print the entire board for easy difficulty
                printf("%c ", board[i][j]);
            } else {
                // For hard difficulty, only print * and ~
                if (board[i][j] == '*' || board[i][j] == '~') {
                    printf("%c ", board[i][j]);
                } else {
                    printf("~ ");  // Print two spaces for other characters
                }
            }
        }
        printf("\n");
    }
}

void displayBoard(Player *player) {
    printf("\n%s's Board:\n", player->name);
    printf("   A B C D E F G H I J\n");
    for (int i = 0; i < GRID_SIZE; i++) {
        printf("%2d ", i + 1);
        for (int j = 0; j < GRID_SIZE; j++) {
            printf("%c ", player->board[i][j]);
        }
        printf("\n");
    }
}

int column_to_index(char column) {
    column = toupper(column);
    if (column < 'A' || column > 'J') {
        return -1; // Signal invalid column
    }
    return column - 'A';
}

// Function to validate ship placement on the grid
int checkShipOverlap(Player *player, Ship *ship, int startRow, int startCol, char orientation) { //tested
    int row = startRow;
    int col = startCol;

    for (int i = 0; i < ship->size; i++) {
        // Check for overlap with other ships
        if (player->board[row][col] != '~') {
            printf("Ship placement overlaps with another ship.\n");
            return 0;
        }

        // Move to the next cell based on orientation
        if (orientation == 'H') {
            col++;
        } else {
            row++;
        }
    }
    return 1; // Placement is valid
}

//place a ship on the player's board
void placeShipOnBoard(Player *player, Ship *ship, int startRow, int startCol, char orientation) { //tested
    int row = startRow;
    int col = startCol;

    for (int i = 0; i < ship->size; i++) {
        // Place the ship identifier on the board
        player->board[row][col] = ship->id;

        // Update the occupied cells in the ship structure
        ship->occupiedCells[i][0] = row;
        ship->occupiedCells[i][1] = col;
        ship->occupiedCells[i][2] = 0; // Set hit status to 0

        // Move to the next cell based on orientation
        if (orientation == 'H') {
            col++;
        } else {
            row++;
        }
    }
    displayBoard(player); 
}

// Function to handle the user input and call validation and placement functions //to be used in main
void placeShips(Player *player) { //tested
    displayBoard(player);
    for (int i = 0; i < TOTALNUMBEROFSHIPS; i++) {
        Ship *ship = &player->ships[i];
        int startRow, startCol;
        char colChar, orientation;
        int valid = 0;
        while (!valid)
        {
            printf("Place your %s (size: %d). Enter starting position (e.g., B3) and orientation (H for horizontal, V for vertical): ", ship->name, ship->size);
            scanf(" %c%d %c", &colChar, &startRow, &orientation);
            orientation = toupper(orientation);

            startCol = column_to_index(colChar);
            startRow--;

            if (startRow < 0 || startRow >= GRID_SIZE || startCol < 0 || startCol >= GRID_SIZE) {
                printf("Invalid coordinates. Please stay within the grid (A1 to J10).\n");
                continue;
            }
            if (orientation != 'H' && orientation != 'V') {
                printf("Invalid orientation. Use 'H' for horizontal or 'V' for vertical.\n");
                continue;
            }

            if (orientation == 'H' && startCol + ship->size > GRID_SIZE) {
                printf("Ship would extend beyond the right edge. Try a different position.\n");
                continue;
            }
            if (orientation == 'V' && startRow + ship->size > GRID_SIZE) {
                printf("Ship would extend beyond the bottom edge. Try a different position.\n");
                continue;
            }
            
            if (checkShipOverlap(player, ship, startRow, startCol, orientation)) {
                placeShipOnBoard(player, ship, startRow, startCol, orientation);
                printf("%s placed successfully.\n", ship->name);
                valid = 1;
            } else {
                printf("Invalid placement. Try again.\n");
            }
        }
    }

    // Clear console and display confirmation
    void clear_screen();
    printf("All ships placed for %s.\n", player->name);
}



// Function to check if the move is "Fire"
int is_fire(char* moveType) {
    return is_equal(moveType, "fire");
}

// Function to check if the move is "Artillery"
int is_artillery(char* moveType) {
    return is_equal(moveType, "artillery");
}

// Function to check if the move is "Torpedo"
int is_torpedo(char* moveType) {
    return is_equal(moveType, "torpedo");
}

// Function to check if the move is "Radar"
int is_radar(char* moveType) {
    return is_equal(moveType, "radar");
}

// Function to check if the move is "Smoke"
int is_smoke(char* moveType) {
    return is_equal(moveType, "smoke");
}
//checking if move inputs are okay
int is_equal(char* str1, char* str2) {
    char lowerStr1[20], lowerStr2[20];

    // Convert both strings to lowercase
    to_lowercase(str1, lowerStr1);
    to_lowercase(str2, lowerStr2);

    return (strcmp(lowerStr1, lowerStr2) == 0); // Compare the lowercase strings
}

// Converts a string to lowercase and stores it in the destination
void to_lowercase(char* src, char* dest) {
    for (int i = 0; src[i] != '\0'; i++) {
        dest[i] = tolower((unsigned char)src[i]);
    }
    dest[strlen(src)] = '\0';
}

void FireMove(Player* attacker, Player* defender, int x, int y, char game_difficulty){ //single cell
    HitOrMiss(attacker,defender,x, y, 'F', 'H', game_difficulty);
}
void ArtilleryMove(Player* attacker, Player* defender, int x, int y, char game_difficulty){ //2x2 area
    HitOrMiss(attacker,defender,x,y,'A','H', game_difficulty);
}
void TorpedoMove(Player* attacker, Player* defender, int x, int y, char game_difficulty){ //row or column move
    if (x==0 && y!=0) //it was a column move
    {
        HitOrMiss(attacker, defender, y, y,'T','V', game_difficulty);
    }else if (y==0 && x!=0) //it was a row move
    {
        HitOrMiss(attacker, defender, x, x,'T','H', game_difficulty);
    }
}
void RadarMove(Player *attacker, Player *defender, int x, int y){
    int shipsFound = 0;

    // Check the 2x2 area for ships
    for (int i = 0; i <= 1; i++) {
        for (int j = 0; j <= 1; j++) {
            if (defender->board[x+i][y+j] != '~' && defender->obscuredArea[x+i][y+j]!='S') { //there is a ship that is not obscured
                shipsFound = 1;
            }
        }
    }

    if (shipsFound) {
        printf("Enemy ships found.\n");
    } else {
        printf("No enemy ships found.\n");
    }

}
void SmokeMove(Player *attacker, int x, int y){
    // Mark the 2x2 area as obscured
    for (int i = 0; i <= 1; i++) {
        for (int j = 0; j <= 1; j++) {
            attacker->obscuredArea[x + i][y + j] = 'S'; // Marking as obscured
        }
    }
    void clear_screen();
    printf("Obscured successfully!");
}

void selectMove(Player *attacker, Player *defender, char game_difficulty) {
     char moveType[20];
    char coordinate[5] = {0}; 
    int x = -1, y = -1;
    int validMove=0;

    printf("Opponent's Grid:\n");
    display_opponent_grid(defender->hits, game_difficulty);
    printf("%s's turn\n", attacker->name);
    displayAvailableMoves();

    while (!validMove) {
        // Reset input
        moveType[0] = '\0';
        coordinate[0] = '\0';

        printf("\nEnter your move: ");
        
        // Read the entire line, trimming newline
        char input[30];
        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("Input error. Try again.\n");
            continue;
        }
        input[strcspn(input, "\n")] = '\0';

        // Validate total input length
        if (strlen(input) > 15) {
            printf("Input too long. Use format 'MoveType Coordinate' (e.g., Fire A1).\n");
            continue;
        }

        // Parse input
        if (sscanf(input, "%19s %4s", moveType, coordinate) != 2) {
            printf("Invalid input. Use format 'MoveType Coordinate' (e.g., Fire A1).\n");
            continue;
        }

        // Special handling for Torpedo move
        if (is_torpedo(moveType)) {
            // Check Torpedo availability
            if (attacker->numOfTorpedo != 1) {
                printf("You are not allowed to use Torpedo!\n");
                continue;
            }

            // Validate Torpedo coordinate
            int coordLen = strlen(coordinate);
            if (coordLen != 1 || 
                (!isalpha(coordinate[0]) && !isdigit(coordinate[0]))) {
                printf("Torpedo move must be a single column letter (A-J) or row number (1-10).\n");
                continue;
            }

            // Convert Torpedo coordinate
            if (isalpha(coordinate[0])) {
                y = column_to_index(coordinate[0]); 
                x = 0; // Entire row
            } else {
                x = atoi(coordinate) - 1; // Entire column
                y = 0;
            }

            // Validate grid boundaries for Torpedo
            if (x < 0 || y < 0 || x >= GRID_SIZE || y >= GRID_SIZE) {
                printf("Torpedo coordinates out of bounds.\n");
                continue;
            }

            // Execute Torpedo move
            TorpedoMove(attacker, defender, x, y, game_difficulty);
            attacker->numOfTorpedo = 0;
            playerswitch(attacker, defender);
            validMove=1;
        } 
        // Other move types
        else {
            // Validate coordinate format for non-Torpedo moves
            int coordLen = strlen(coordinate);
            if (coordLen < 2 || coordLen > 3 || 
                !isalpha(coordinate[0]) || 
                (coordLen == 3 && (!isdigit(coordinate[1]) || !isdigit(coordinate[2]))) ||
                (coordLen == 2 && !isdigit(coordinate[1]))) {
                printf("Invalid coordinate. Use format like A1 or A10.\n");
                continue;
            }

            // Convert coordinate
            char colChar = coordinate[0];
            y = column_to_index(colChar);
            
            // Handle coordinate conversion based on length
            if (coordLen == 2) {
                x = atoi(&coordinate[1]) - 1;  // Use atoi for proper conversion
            } else if (coordLen == 3) {
                x = atoi(&coordinate[1]) - 1;  // Handles both single and double-digit rows
            } 

            // Validate grid boundaries
            if (x < 0 || y < 0 || x >= GRID_SIZE || y >= GRID_SIZE) {
                printf("Coordinates out of bounds.\n");
                continue;
            }
            // Select move based on input
            if (is_fire(moveType)) {
                FireMove(attacker, defender, x, y, game_difficulty);
                validMove = 1;
            }else if (is_artillery(moveType)) {
                if (attacker->numOfArtillery ==1)
                {
                    int shipsSunkenBefore = defender->numOfShipsSunken;

                    // Perform Artillery Move
                    ArtilleryMove(attacker, defender, x, y, game_difficulty);
        
                    // Check if a ship was sunk during this move
                    if (defender->numOfShipsSunken > shipsSunkenBefore) {
                        // Ship sunk, keep Artillery available for next turn
                        attacker->numOfArtillery = 1;
                    } else {
                        // No ship sunk, disable Artillery
                        attacker->numOfArtillery = 0;
                    }
                    validMove=1;
                
                }else{
                    printf("You cannot use Artillery!\n");
                }          
            } else if (is_radar(moveType)){
                if (attacker->numOfRadars > 0) {
                    RadarMove(attacker, defender, x, y);
                    attacker->numOfRadars--;
                    validMove = 1;
                } else {
                    printf("You used up your radar attempts.You lost your turn\n");
                    validMove=1;
                }
            } else if (is_smoke(moveType)){
                if (attacker->numOfShipsSunken-attacker->numOfSmokeScreensPerformed >0){
                if (x < 0 || y < 0 || x + 1 >= GRID_SIZE || y + 1 >= GRID_SIZE) {
                    printf("Invalid coordinates for smoke screen.\n");
                    continue;
                }
                SmokeMove(attacker, x, y);
                attacker->numOfSmokeScreensPerformed++;
                validMove = 1;
                }else{
                    printf("You exceeded your smoke attempts. You've lost your turn!\n");
                    validMove=1;
                }   
            } else {
                printf("Invalid move\n");
            }
        }
        if (validMove)
        {
            playerswitch(attacker, defender);
        }
        
    }
}


void HitOrMiss(Player *attacker, Player *defender, int x, int y, char movetype, char orientation, char game_difficulty) {
    markAffectedArea(x, y, movetype, orientation);
    int HitRegister=0;
    
    int previouslySunk[TOTALNUMBEROFSHIPS] = {0};
    for (int k = 0; k < TOTALNUMBEROFSHIPS; k++) {
        previouslySunk[k] = isShipSunk(&defender->ships[k]);
    }

    // Iterate over the affected area to check for hits
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            if (affectedArea[i][j] == 'X') {           // Affected
                char shipID = defender->board[i][j];   // Get the ship identifier
                if (shipID != '~') {                    // Check if there's a ship
                    // Check if the cell has already been hit
                    if (defender->hits[i][j] != '*') { //* means it was hit in a previous turn
                        defender->hits[i][j] = '*';
                        // Find the ship using its ID directly
                        for (int k = 0; k < TOTALNUMBEROFSHIPS; k++) {
                            if (defender->ships[k].id == shipID) {
                                // update the hit status in occupiedCells
                                for (int m = 0; m < defender->ships[k].size; m++) {
                                    if (defender->ships[k].occupiedCells[m][0] == i &&
                                        defender->ships[k].occupiedCells[m][1] == j) {
                                        defender->ships[k].occupiedCells[m][2] = 1; // Mark as hit
                                        break;
                                    }
                                }
                                HitRegister++;
                                break; 
                            }
                        }
                    }
                } else {
                    // water, mark as miss
                    if (defender->hits[i][j] == '~') {
                        defender->hits[i][j] = 'o'; // Mark as miss
                    }
                }
            }
        }
    }
    display_opponent_grid(defender->hits,game_difficulty);
    HitOrMissMessageDisplay(HitRegister > 0 ? 1 : 0);
    for (int k = 0; k < TOTALNUMBEROFSHIPS; k++) {
        if (!previouslySunk[k] && isShipSunk(&defender->ships[k])) {
            printf("%s has been sunk!\n", defender->ships[k].name);
            defender->numOfShipsSunken++;
            attacker->numOfArtillery = 1;

            if (defender->numOfShipsSunken == 3) {
                attacker->numOfTorpedo = 1;
            }
        }
    }
}

void markAffectedArea(int x, int y, char moveType, char orientation) { //this function is ONLY for HITS so far.
                                                                     //validation of placement for a move is NOT accounted for as they are checked in each move's respective function.
    // Reset affected grid for new move incoming                     //radar sweep and smoke screen are NOT accounted for as they do not inflict direct hits.
    for (int i = 0; i <= GRID_SIZE; i++){                            //direction is only to account for the column/row move.
        for (int j = 0; j <= GRID_SIZE; j++)
            affectedArea[i][j] = '~';
    }
        
    // Mark the affected cells based on move type, Note that X is for affected
    if (moveType == 'F') { //sinle cell
        affectedArea[x][y]='X'; 
    }
    else if (moveType == 'A') { //2x2 area
        affectedArea[x][y] = 'X';
        affectedArea[x + 1][y] = 'X';
        affectedArea[x][y + 1] = 'X';
        affectedArea[x + 1][y + 1] = 'X';
    }
    else if (moveType == 'T') {
        if (orientation == 'H') { //row move
            for (int i = 0; i < GRID_SIZE; i++) {
                affectedArea[x][i] = 'X';
            }
        } else if (orientation == 'V') { //column move
            for (int i = 0; i < GRID_SIZE; i++) {
                affectedArea[i][y] = 'X';
            }
        }
    }
}

// Function to check if the ship is sunk. 0 is fail 1 is success
int isShipSunk(Ship *ship) { 
    for (int i = 0; i < ship->size ; i++) {
        if (ship->occupiedCells[i][2] == 0)
        {
            return 0;
        }
        
    }
    return 1; // All parts are hit
}
void HitOrMissMessageDisplay(int movesuccess){ //after any move is completed, 
    if (movesuccess==0) //MISS                 //pass the message success or not to display output 
    {                                          
       printf("Miss!\n");
    }
    if (movesuccess==1)//HIT
    {
        printf("Hit!\n");
    }
}
void stringcopy(char* dest,char* src) {
    while (*src) {
        *dest++ = *src++;
    }
    *dest = '\0';
}

