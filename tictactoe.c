#include <gtk/gtk.h>
#include <stdbool.h>
#include <limits.h>

// Global variables
char grid[3][3];
GtkWidget *buttons[3][3];
GtkWidget *status_label;

// Initialize the grid
void initializeGrid() {
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            grid[i][j] = ' ';
        }
    }
}

// Check for a winner
bool checkWinner(char player) {
    for (int i = 0; i < 3; i++) {
        if ((grid[i][0] == player && grid[i][1] == player && grid[i][2] == player) ||
            (grid[0][i] == player && grid[1][i] == player && grid[2][i] == player)) {
            return true;
        }
    }
    if ((grid[0][0] == player && grid[1][1] == player && grid[2][2] == player) ||
        (grid[0][2] == player && grid[1][1] == player && grid[2][0] == player)) {
        return true;
    }
    return false;
}

// Check if the game is a draw
bool isDraw() {
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (grid[i][j] == ' ') return false;
        }
    }
    return true;
}

// Minimax algorithm
int minimax(bool isMaximizing) {
    if (checkWinner('O')) return 10;
    if (checkWinner('X')) return -10;
    if (isDraw()) return 0;

    int bestScore = isMaximizing ? INT_MIN : INT_MAX;

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (grid[i][j] == ' ') {
                grid[i][j] = isMaximizing ? 'O' : 'X';
                int score = minimax(!isMaximizing);
                grid[i][j] = ' ';
                bestScore = isMaximizing
                           ? (score > bestScore ? score : bestScore)
                           : (score < bestScore ? score : bestScore);
            }
        }
    }
    return bestScore;
}

// Find the best move for the AI
void findBestMove() {
    int bestScore = INT_MIN;
    int moveRow = -1, moveCol = -1;

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (grid[i][j] == ' ') {
                grid[i][j] = 'O';
                int score = minimax(false);
                grid[i][j] = ' ';

                if (score > bestScore) {
                    bestScore = score;
                    moveRow = i;
                    moveCol = j;
                }
            }
        }
    }
    grid[moveRow][moveCol] = 'O';
    gtk_button_set_label(GTK_BUTTON(buttons[moveRow][moveCol]), "O");
    gtk_widget_set_sensitive(buttons[moveRow][moveCol], FALSE);
}

// Reset the game
static void reset_game(GtkWidget *widget, gpointer data) {
    initializeGrid();
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            gtk_button_set_label(GTK_BUTTON(buttons[i][j]), "");
            gtk_widget_set_sensitive(buttons[i][j], TRUE);
        }
    }
    gtk_label_set_text(GTK_LABEL(status_label), "Your turn! Click any square.");
}

// Structure to store button coordinates
typedef struct {
    int row;
    int col;
} ButtonCoords;

// Handle button clicks
static void button_clicked(GtkWidget *widget, ButtonCoords *coords) {
    int row = coords->row;
    int col = coords->col;

    // Player's move
    grid[row][col] = 'X';
    gtk_button_set_label(GTK_BUTTON(widget), "X");
    gtk_widget_set_sensitive(widget, FALSE);

    // Check if player wins
    if (checkWinner('X')) {
        gtk_label_set_text(GTK_LABEL(status_label), "You win!");
        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 3; j++)
                gtk_widget_set_sensitive(buttons[i][j], FALSE);
        return;
    }

    // Check for draw
    if (isDraw()) {
        gtk_label_set_text(GTK_LABEL(status_label), "It's a draw!");
        return;
    }

    // Computer's move
    gtk_label_set_text(GTK_LABEL(status_label), "Computer is thinking...");
    
    // Process pending events to update the UI
    while (g_main_context_iteration(NULL, FALSE));
    
    findBestMove();

    // Check if computer wins
    if (checkWinner('O')) {
        gtk_label_set_text(GTK_LABEL(status_label), "Computer wins!");
        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 3; j++)
                gtk_widget_set_sensitive(buttons[i][j], FALSE);
        return;
    }

    // Check for draw again
    if (isDraw()) {
        gtk_label_set_text(GTK_LABEL(status_label), "It's a draw!");
        return;
    }

    gtk_label_set_text(GTK_LABEL(status_label), "Your turn!");
}

static void activate(GtkApplication *app, gpointer user_data) {
    // Create main window
    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Tic-tac-toe");
    gtk_window_set_default_size(GTK_WINDOW(window), 300, 400);

    // Create main vertical box
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_window_set_child(GTK_WINDOW(window), vbox);

    // Create grid
    GtkWidget *grid_widget = gtk_grid_new();
    gtk_widget_set_margin_start(grid_widget, 10);
    gtk_widget_set_margin_end(grid_widget, 10);
    gtk_widget_set_margin_top(grid_widget, 10);
    gtk_widget_set_margin_bottom(grid_widget, 10);
    gtk_box_append(GTK_BOX(vbox), grid_widget);

    // Create buttons with coordinates
    ButtonCoords *coords_array = g_malloc(sizeof(ButtonCoords) * 9);
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            buttons[i][j] = gtk_button_new();
            gtk_widget_set_size_request(buttons[i][j], 80, 80);
            gtk_grid_attach(GTK_GRID(grid_widget), buttons[i][j], j, i, 1, 1);
            
            // Store coordinates
            coords_array[i * 3 + j].row = i;
            coords_array[i * 3 + j].col = j;
            
            g_signal_connect(buttons[i][j], "clicked", G_CALLBACK(button_clicked), 
                           &coords_array[i * 3 + j]);
        }
    }

    // Create status label
    status_label = gtk_label_new("Your turn! Click any square.");
    gtk_box_append(GTK_BOX(vbox), status_label);

    // Create reset button
    GtkWidget *reset_button = gtk_button_new_with_label("Reset Game");
    gtk_widget_set_margin_start(reset_button, 10);
    gtk_widget_set_margin_end(reset_button, 10);
    gtk_widget_set_margin_bottom(reset_button, 10);
    g_signal_connect(reset_button, "clicked", G_CALLBACK(reset_game), NULL);
    gtk_box_append(GTK_BOX(vbox), reset_button);

    // Initialize game
    initializeGrid();

    // Present window
    gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char **argv) {
    GtkApplication *app = gtk_application_new("org.gtk.tictactoe", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}