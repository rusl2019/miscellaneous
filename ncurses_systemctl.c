#include <ncurses.h>
#include <stdlib.h>
#include <string.h>

void print_menu(WINDOW *menu_win, int highlight, char *choices[], int n_choices,
		int *sel_services, int *srv_status)
{
	int x = 2, y = 2, i;
	box(menu_win, 0, 0);
	for (i = 0; i < n_choices; ++i) {
		if (highlight == i + 1) {
			wattron(menu_win, A_REVERSE);
			if (i < n_choices - 2) { // Exclude Help and Exit
				if (srv_status[i] == -1) {
					mvwprintw(menu_win, y, x, "[-] %s",
						  choices[i]);
				} else {
					mvwprintw(menu_win, y, x, "[%c] %s",
						  sel_services[i] ? 'x' : ' ',
						  choices[i]);
				}
			} else {
				mvwprintw(menu_win, y, x, "%s", choices[i]);
			}
			wattroff(menu_win, A_REVERSE);
		} else {
			if (i < n_choices - 2) { // Exclude Help and Exit
				if (srv_status[i] == -1) {
					mvwprintw(menu_win, y, x, "[-] %s",
						  choices[i]);
				} else {
					mvwprintw(menu_win, y, x, "[%c] %s",
						  sel_services[i] ? 'x' : ' ',
						  choices[i]);
				}
			} else {
				mvwprintw(menu_win, y, x, "%s", choices[i]);
			}
		}
		++y;
	}
	wrefresh(menu_win);
}

int get_service_status(char *service)
{
	char cmd[256];
	snprintf(cmd, sizeof(cmd), "systemctl is-active --quiet %s", service);
	int result = system(cmd);
	if (result == 0) {
		return 1; // Active
	} else if (result == 1024) {
		return -1; // Service not found
	} else {
		return 0; // Inactive
	}
}

void handle_action(char *service, char *action, char *password)
{
	int status = get_service_status(service);
	if (status == -1) {
		char msg[256];
		snprintf(msg, sizeof(msg), "%s service not found", service);
		show_message(msg);
		return;
	}

	char cmd[256];
	snprintf(cmd, sizeof(cmd), "echo %s | sudo -S systemctl %s %s",
		 password, action, service);
	int result = system(cmd);
	if (result == 0) {
		char msg[256];
		snprintf(msg, sizeof(msg), "%s service %sed successfully",
			 service, action);
		show_message(msg);
	} else {
		char msg[256];
		snprintf(msg, sizeof(msg), "Failed to %s %s service", action,
			 service);
		show_message(msg);
	}
}

void get_password(char *password)
{
	WINDOW *pass_win = newwin(5, 50, (LINES - 5) / 2, (COLS - 50) / 2);
	box(pass_win, 0, 0);
	mvwprintw(pass_win, 1, 1, "Enter sudo password: ");
	wrefresh(pass_win);

	int ch, i = 0;
	noecho();
	keypad(pass_win, TRUE);
	while ((ch = wgetch(pass_win)) != '\n' && ch != '\r' && i < 255) {
		if (ch == KEY_BACKSPACE || ch == 127) {
			if (i > 0) {
				i--;
				mvwprintw(pass_win, 1, 22 + i, " ");
				wmove(pass_win, 1, 22 + i);
				wrefresh(pass_win);
			}
		} else {
			password[i++] = ch;
			mvwprintw(pass_win, 1, 22 + i - 1, "*");
			wrefresh(pass_win);
		}
	}
	password[i] = '\0';
	delwin(pass_win);
}

void print_help()
{
	int help_width = 50;
	int help_height = 10;
	int startx = (COLS - help_width) / 2;
	int starty = (LINES - help_height) / 2;
	WINDOW *help_win = newwin(help_height, help_width, starty, startx);
	box(help_win, 0, 0);
	mvwprintw(help_win, 1, 1, "Help Menu:");
	mvwprintw(help_win, 2, 1, "Use 'j' to move down");
	mvwprintw(help_win, 3, 1, "Use 'k' to move up");
	mvwprintw(help_win, 4, 1, "Use 'q' to exit");
	mvwprintw(help_win, 5, 1, "Use 'space' to toggle service");
	mvwprintw(help_win, 6, 1, "Press Enter to confirm selection");
	mvwprintw(help_win, 8, 1, "Press any key to go back");
	wrefresh(help_win);
	getch();
	delwin(help_win);
	clear();
}

void show_message(const char *message)
{
	int msg_width = strlen(message) + 4;
	int msg_height = 5;
	int startx = (COLS - msg_width) / 2;
	int starty = (LINES - msg_height) / 2;
	WINDOW *msg_win = newwin(msg_height, msg_width, starty, startx);
	box(msg_win, 0, 0);
	mvwprintw(msg_win, 2, 2, "%s", message);
	wrefresh(msg_win);
	getch();
	delwin(msg_win);
}

int main()
{
	// Item menu
	char *services[] = {"httpd", "nginx", "mysql", "Help", "Exit"};
	int n_services = sizeof(services) / sizeof(char *);

	int highlight = 1;
	int choice = 0;
	int c;
	int srv_status[3];
	int sel_services[3];

	// Update status
	for (int i = 0; i < n_services - 2; ++i) { // Exclude Help and Exit
		srv_status[i] = get_service_status(services[i]);
		sel_services[i] = srv_status[i];
	}

	initscr();
	clear();
	noecho();
	cbreak(); // Line buffering disabled, Pass on everything to me

	int startx = (COLS - 40) / 2;
	int starty = (LINES - 15) / 2;

	WINDOW *menu_win = newwin(15, 40, starty, startx);
	keypad(menu_win, TRUE);
	refresh();
	print_menu(menu_win, highlight, services, n_services, sel_services,
		   srv_status);

	char password[256] = "";
	get_password(password);

	clear();
	refresh();
	print_menu(menu_win, highlight, services, n_services, sel_services,
		   srv_status);
	refresh();

	while (1) {
		c = wgetch(menu_win);
		switch (c) {
		case 'k':
			if (highlight == 1)
				highlight = n_services;
			else
				--highlight;
			break;
		case 'j':
			if (highlight == n_services)
				highlight = 1;
			else
				++highlight;
			break;
		case 'q':
			choice = n_services; // Set choice to "Exit"
			break;
		case ' ':
			if (highlight <=
			    n_services -
				2) { // Toggle selection, Exclude Help and Exit
				if (srv_status[highlight - 1] !=
				    -1) { // Only toggle if service is found
					sel_services[highlight - 1] =
					    !sel_services[highlight - 1];
					if (sel_services[highlight - 1]) {
						handle_action(
						    services[highlight - 1],
						    "start", password);
					} else {
						handle_action(
						    services[highlight - 1],
						    "stop", password);
					}
				}
			}
			break;
		case 10: // Enter
			choice = highlight;
			if (strcmp(services[choice - 1], "Help") == 0) {
				print_help();
				clear();
				refresh();
				print_menu(menu_win, highlight, services,
					   n_services, sel_services,
					   srv_status);
				refresh();
				choice = 0;
			}
			break;
		default:
			refresh();
			break;
		}
		print_menu(menu_win, highlight, services, n_services,
			   sel_services, srv_status);
		refresh();
		if (choice == n_services) // User chose to exit, exit the loop
			break;
	}

	show_message("Exiting program");
	endwin();
	return 0;
}