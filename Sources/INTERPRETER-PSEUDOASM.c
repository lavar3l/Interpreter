//INTERPRETER PSEUDOASSEMBLERA
//AUTOR: PRZEMYSŁAW DOMINIKOWSKI
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#ifdef _MSC_VER
#pragma warning(disable:4996)
#pragma warning(disable:4018)
#endif
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#define INT_SIZE 4
#define INT_LENGTH 11
#define INF 1000000000 //OZNACZENIE WARTOŚCI NIEZAINICJALIZOWANEJ
#define DEFAULT_STR "                              " //PUSTY STRING DO REZERWACJI PAMIĘCI
#define NOT_FOUND -1
#define LAST_LINE -1
#define COLUMN_MARGIN 30
#define EPSILON 5

//#######################         PARAMETRY		     ############################

#define MAX_LINE 60 //MAKSYMALNA DŁUGOŚĆ LINII KODU PSEUDOASSEMBLERA
#define MAX_WORD 20 //MAKSYMALNA DŁUGOŚĆ ETYKIETY I SŁOWA
#define REG_NUM 16 //LICZBA REJESTRÓW

//#######################     PROTOTYPY FUNKCJI      ############################

//FUNKCJE PSEUDOASSEMBLERA
void do_A(int loc1, int loc2);
void do_S(int loc1, int loc2);
void do_M(int loc1, int loc2);
void do_D(int loc1, int loc2);
void do_C(int loc1, int loc2);
void do_J(int loc1);
void do_JZ(int loc1);
void do_JP(int loc1);
void do_JN(int loc1);
void do_L(int loc1, int loc2);
void do_LA(int loc1, int loc2);
void do_ST(int loc1, int loc2);
void ChangeResultBit(int result_of_operation);

//FUNCKJE PRZYGOTOWANIA PAMIĘCI
int CountRequiredMemory(void);
void PrepareDynamicTables(void);
void PrepareRegisters(void);
void PrepareMemory(void);
void PrepareLabels(void);
void PrepareOrders(void);

//FUNKCJE DEKLAROWANIA/WYSZUKIWANIA SŁÓW I ETYKIET
int AddWord(int position, char* item);
void AddLabel(int position, char* item);
int FindLabel(char* label);
int FindWord(char* label, int* word_size);

//POMOCNICZE FUNKCJE DO PRZETWORZENIA KODU PSEUDOASSEMBLERA 
void ParseLine(char asm_line[MAX_LINE]);
void FixImportedTxt(void);
int ItemType(char* item);
int IsNumber(char* item);
int EmptyString(char* item);

//GŁÓWNE FUNKCJE INTERPRETERA PSEUDOASSEMBLERA
void MainLoop(void);
void ExecuteCommand(char asm_line[MAX_LINE], int current_line);

//FUNKCJE ODPOWIEDZIALNE ZA GRAFICZNĄ PREZENTACJĘ WYNIKÓW / KOMUNIKACJĘ Z UŻYTKOWNIKIEM
void UserPreDialog(void);
void PresentProgramStatus(int current_line);
void do_READ(int loc1, int size, char* label);
void do_WRITE(int loc1, int size, char* label);
void SetConsoleModeToColor(void);
void RestoreCursorPosition(void);
void ShowCredit(void);
void ShowError(int current_line, int problem_type, char* err_message);
void ShowWarning(char* war_message);


//#################     DEKLARACJE ZMIENNYCH ZEWNĘTRZNYCH     ####################

//USTAWIENIA PROGRAMU
int RULE_linebyline = 0, RULE_showallmemory = 0;

//OBSŁUGA PLIKÓW TEKSTOWYCH
FILE* txtinput;
int max_length1_FIX, max_length2_FIX;

//OBSŁUGA PAMIĘCI - LICZNIKI
int memory_lines = 0;
int order_lines = 0;
int input_lines = 0;
int elements = 0;
int memory_size = REG_NUM;
int word_progress = 0;
int label_progress = 0;

//OBSŁUGA PAMIĘCI - STRUKTURY
int* memory, * prev_memory;
struct map
{
	char key[MAX_WORD];
	int location;
	int size;
	int active;
} *labels, * words;
struct input_from_txt
{
	char asm[MAX_LINE];
} *orders;
struct fixed_input_from_txt
{
	char label[MAX_WORD];
	char command[MAX_WORD];
	char arg[2 * MAX_WORD];
} *colourful_orders;

//REJESTR STANU OSTATNIEJ OPERACJI
char last_result_bit[] = "11";
int  next_command_reg = 0;

//OBSŁUGA WYPISYWANIA
int touched_element_loc = NOT_FOUND;
int type_of_touch = 0;

//OBSŁUGA BŁĘDÓW I OSTRZEŻEŃ
int is_warning = 0;
char* warning;

//##########################     FUNKCJA MAIN      ###############################

int main(int argc, char** argv)
{
	char code_file[MAX_LINE];
	errno_t err_input;

	SetConsoleModeToColor();
	ShowCredit();

	//PROCEDURA OTWARCIA PLIKÓW TEKSTOWYCH
	printf("\x1b[1;4mPodaj nazwę pliku\x1b[m z kodem pseudoassemblera: ");
	printf("\x1b[91m");
	scanf_s("%s", code_file, (unsigned)_countof(code_file));
	printf("\x1b[0m");

	err_input = fopen_s(&txtinput, code_file, "r");
	if (err_input != 0)
	{
		printf("Próba otwarcia pliku \x1b[91m%s\x1b[0m zakończyła się niepowodzeniem.\n", code_file);
		printf("Interpreter zatrzyma swoje działanie.\n");
		system("PAUSE");
		return (-1);
	}
	UserPreDialog();

	//PRZETWORZENIE KODU W PSEUDOASSEMBLERZE
	PrepareDynamicTables();
	PrepareRegisters();
	PrepareMemory();
	PrepareLabels();
	PrepareOrders();
	FixImportedTxt();

	//URUCHOMIENIE PROGRAMU
	MainLoop();

	//ZAMKNIĘCIE INTERPRETERA
	fclose(txtinput);

	free(memory);
	free(prev_memory);
	free(labels);
	free(words);
	free(orders);
	free(colourful_orders);

	return 0;
} //main

//###################      FUNKCJE PSEUDOASSEMBLERA      #########################

void do_A(int loc1, int loc2)
{
	memory[loc1] += memory[loc2];
	ChangeResultBit(memory[loc1]);
	next_command_reg++;
	return;
} //do_A

void do_S(int loc1, int loc2)
{
	memory[loc1] -= memory[loc2];
	ChangeResultBit(memory[loc1]);
	next_command_reg++;
	return;
} //do_S

void do_M(int loc1, int loc2)
{
	memory[loc1] *= memory[loc2];
	ChangeResultBit(memory[loc1]);
	next_command_reg++;
	return;
} //do_M

void do_D(int loc1, int loc2)
{
	if (memory[loc2] == 0)
	{
		strcpy(last_result_bit, "11");
		ShowWarning("Usiłowano wykonać \ndzielenie przez 0.");
	}
	else
	{
		memory[loc1] /= memory[loc2];
		ChangeResultBit(memory[loc1]);
	}
	next_command_reg++;
	return;
} //do_D

void do_C(int loc1, int loc2)
{
	ChangeResultBit(memory[loc1] - memory[loc2]);
	next_command_reg++;
	return;
} //do_C

void do_J(int loc1)
{
	next_command_reg = loc1;
	return;
} //do_J

void do_JZ(int loc1)
{
	if (strcmp(last_result_bit, "00") == 0) next_command_reg = loc1;
	else next_command_reg++;
	return;
} //do_JZ

void do_JP(int loc1)
{
	if (strcmp(last_result_bit, "01") == 0) next_command_reg = loc1;
	else next_command_reg++;
	return;
} //do_JP

void do_JN(int loc1)
{
	if (strcmp(last_result_bit, "10") == 0) next_command_reg = loc1;
	else next_command_reg++;
	return;
} //do_JN

void do_L(int loc1, int loc2)
{
	memory[loc1] = memory[loc2];
	next_command_reg++;
	return;
} //do_L

void do_LA(int loc1, int loc2)
{
	memory[loc1] = INT_SIZE * (loc2 - REG_NUM);
	next_command_reg++;
	return;
} //do_LA

void do_ST(int loc1, int loc2)
{
	memory[loc2] = memory[loc1];
	next_command_reg++;
	return;
} //do_ST

void ChangeResultBit(int result_of_operation)
{
	if (result_of_operation == 0) strcpy(last_result_bit, "00");
	else if (result_of_operation > 0) strcpy(last_result_bit, "01");
	else if (result_of_operation < 0) strcpy(last_result_bit, "10");
	return;
} //ChangeResultBit

//###################      FUNKCJE PRZYGOTOWANIA PAMIĘCI      ####################

int CountRequiredMemory(void)
{
	int current_line = 0, current_pos = 0, current_item, item_type, new_tab_size = 0;
	char asm_line[MAX_LINE], * item, * arg1 = DEFAULT_STR, * arg2 = DEFAULT_STR, * arg3 = DEFAULT_STR;
	rewind(txtinput);
	while (fgets(asm_line, MAX_LINE, txtinput) && current_line < memory_lines)
	{
		if (EmptyString(asm_line) == 1) continue;
		current_item = 0;
		item = strtok(asm_line, " ,()*<>	\n");

		while (item != NULL)
		{
			item_type = ItemType(item);
			if (current_item == 0 && item_type != 0) current_item = 1;
			if (current_item == 2) arg1 = item;
			if (current_item == 3) arg2 = item;
			if (current_item == 4) arg3 = item;
			current_item++;
			item = strtok(NULL, " ,()*<>	\n");
		}

		if (strcmp(arg2, "INT") == 0 || strcmp(arg2, "INTEGER") == 0) //TWORZYMY TABLICĘ
		{
			new_tab_size = atoi(arg1);
			current_pos += new_tab_size;
		}
		else current_pos++;

		current_line++;
	}
	return current_pos;
} //CountRequiredMemory

void PrepareDynamicTables(void)
{
	int code_lines = 0, tmp_memory_size = 0;
	char asm_line[MAX_LINE];
	rewind(txtinput);
	while (fgets(asm_line, MAX_LINE, txtinput))
	{
		if (EmptyString(asm_line) == 1) continue;
		code_lines++;
		ParseLine(asm_line);
	}
	tmp_memory_size = CountRequiredMemory();
	memory = (int*)malloc((REG_NUM + tmp_memory_size) * sizeof(*memory));
	prev_memory = (int*)malloc((REG_NUM + tmp_memory_size) * sizeof(*prev_memory));
	labels = calloc(order_lines, sizeof(struct map));
	words = calloc(memory_lines, sizeof(struct map));
	orders = calloc(order_lines + memory_lines, sizeof(struct input_from_txt));
	colourful_orders = calloc(order_lines + memory_lines, sizeof(struct fixed_input_from_txt));
	return;
} //PrepareDynamicTables

void PrepareRegisters(void)
{
	int i;
	for (i = 0; i < REG_NUM; i++) memory[i] = -INF;
	memory[REG_NUM - 2] = 0;
	memory[REG_NUM - 1] = 0;
	return;
} //PrepareRegisters

void PrepareMemory(void)
{
	int i, current_line = 0, current_pos = REG_NUM, current_item, item_type, new_tab_size = 0, new_tab_value = 0, word_position = 0;
	char asm_line[MAX_LINE], * item, * command = DEFAULT_STR, * arg1 = DEFAULT_STR, * arg2 = DEFAULT_STR, * arg3 = DEFAULT_STR;
	rewind(txtinput);
	while (fgets(asm_line, MAX_LINE, txtinput) && current_line < memory_lines)
	{
		if (EmptyString(asm_line) == 1) continue;
		current_item = 0;
		item = strtok(asm_line, " ,()*<>	\n");

		while (item != NULL)
		{
			item_type = ItemType(item);

			if (current_item == 0)
			{
				if (item_type == 0) word_position = AddWord(current_pos, item);
				else current_item = 1;
			}
			if (current_item == 1) command = item;
			if (current_item == 2) arg1 = item;
			if (current_item == 3) arg2 = item;
			if (current_item == 4) arg3 = item;

			current_item++;
			item = strtok(NULL, " ,()*<>	\n");
		}

		if (strcmp(arg2, "INT") == 0 || strcmp(arg2, "INTEGER") == 0) //TWORZYMY TABLICĘ
		{
			new_tab_size = atoi(arg1);
			if (strcmp(command, "DC") == 0) new_tab_value = atoi(arg3);
			else new_tab_value = -INF;
			for (int i = current_pos; i < current_pos + new_tab_size; i++) memory[i] = new_tab_value;
			current_pos += new_tab_size;
			words[word_position].size = new_tab_size;
		}
		else //TWORZYMY ZMIENNĄ
		{
			if (strcmp(command, "DC") == 0) memory[current_pos] = atoi(arg2);
			else memory[current_pos] = -INF;
			current_pos++;
			words[word_position].size = 1;
		}

		current_line++;
	}
	memory_size = current_pos;
	for (i = 0; i < memory_size; i++)
		prev_memory[i] = -INF;
	return;
} //PrepareMemory

void PrepareLabels(void)
{
	int current_line = 0, item_type;
	char asm_line[MAX_LINE], * item;
	rewind(txtinput);
	while (fgets(asm_line, MAX_LINE, txtinput))
	{
		if (EmptyString(asm_line) == 1) continue;
		item = strtok(asm_line, " ,()*<>	\n");
		item_type = ItemType(item);
		if (item_type == 0 && current_line >= memory_lines) AddLabel(current_line, item);
		current_line++;
	}
	return;
} //PrepareLabels

void PrepareOrders(void)
{
	int current_line = 0;
	char asm_line[MAX_LINE];
	rewind(txtinput);
	while (fgets(asm_line, MAX_LINE, txtinput))
	{
		if (EmptyString(asm_line) == 1) continue;
		strcpy(orders[current_line].asm, asm_line);
		current_line++;
	}
	return;
} //PrepareOrders

//#########      FUNKCJE DEKLAROWANIA/WYSZUKIWANIA SŁOW I ETYKIET      ###########

int AddWord(int position, char* item)
{
	strcpy(words[word_progress].key, item);
	words[word_progress].location = position;
	words[word_progress].active = 1;
	word_progress++;
	return (word_progress - 1);
} //AddWord

void AddLabel(int position, char* item)
{
	strcpy(labels[label_progress].key, item);
	labels[label_progress].location = position;
	labels[label_progress].active = 1;
	label_progress++;
	return;
} //AddLabel

int FindLabel(char* label)
{
	int i;
	for (i = 0; i < order_lines; i++)
		if (strcmp(labels[i].key, label) == 0) return labels[i].location;
	return NOT_FOUND;
} //FindLabel

int FindWord(char* label, int* word_size)
{
	int i;
	for (i = 0; i < memory_lines; i++)
		if (strcmp(words[i].key, label) == 0)
		{
			*word_size = words[i].size;
			return words[i].location;
		}
	return NOT_FOUND;
} //FindWord

//#####      POMOCNICZE FUNKCJE DO PRZETWORZENIA KODU PSEUDOASSEMBLERA      ######

void ParseLine(char asm_line[MAX_LINE])
{
	int type_of_item = 0;
	char* item;

	item = strtok(asm_line, " ,()*<>	\n");
	while (item != NULL)
	{
		type_of_item = ItemType(item);
		if (type_of_item >= 3) order_lines++;
		else if (type_of_item >= 1) memory_lines++;
		if (type_of_item == 3 || type_of_item == 4) input_lines++;
		item = strtok(NULL, " ,()*<>	\n");
	}
	return;
} //ParseLine

int ItemType(char* item)
{
	if (strcmp(item, "DC") == 0) return 1;
	else if (strcmp(item, "DS") == 0) return 2;
	else if (strcmp(item, "READ") == 0) return 3;
	else if (strcmp(item, "WRITE") == 0) return 4;
	else if (strcmp(item, "AR") == 0) return 5;
	else if (strcmp(item, "SR") == 0) return 6;
	else if (strcmp(item, "MR") == 0) return 7;
	else if (strcmp(item, "DR") == 0) return 8;
	else if (strcmp(item, "CR") == 0) return 9;
	else if (strcmp(item, "LR") == 0) return 10;
	else if (strcmp(item, "A") == 0) return 11;
	else if (strcmp(item, "S") == 0) return 12;
	else if (strcmp(item, "M") == 0) return 13;
	else if (strcmp(item, "D") == 0) return 14;
	else if (strcmp(item, "C") == 0) return 15;
	else if (strcmp(item, "L") == 0) return 16;
	else if (strcmp(item, "LA") == 0) return 17;
	else if (strcmp(item, "ST") == 0) return 18;
	else if (strcmp(item, "J") == 0) return 19;
	else if (strcmp(item, "JZ") == 0) return 20;
	else if (strcmp(item, "JP") == 0) return 21;
	else if (strcmp(item, "JN") == 0) return 22;
	else return 0; //SŁOWO NIE JEST ROZKAZEM PSEUDOASSEMBLERA
} //ItemType

int IsNumber(char* item)
{
	int i;
	for (i = 0; i < strlen(item); i++)
	{
		if (item[i] < '0' || item[i] > '9') return 0;
	}
	return 1;
} //IsNumber

int EmptyString(char* item)
{
	int i;
	for (i = 0; i < strlen(item); i++)
	{
		if (item[i] >= 'a' && item[i] <= 'z') return 0;
		if (item[i] >= 'A' && item[i] <= 'Z') return 0;
	}
	return 1;
} //EmptyString

void FixImportedTxt(void)
{
	int current_line = 0, current_item, item_type;
	int line_position = 0;
	char asm_line[MAX_LINE], * item, * command = DEFAULT_STR, * arguments = DEFAULT_STR, * label = DEFAULT_STR;
	max_length1_FIX = 0, max_length2_FIX = 0;
	rewind(txtinput);
	while (fgets(asm_line, MAX_LINE, txtinput))
	{
		if (EmptyString(asm_line) == 1) continue;
		current_item = 0;
		item = strtok(asm_line, " 	\n");

		while (item != NULL)
		{
			item_type = ItemType(item);

			if (current_item == 0)
			{
				if (item_type == 0)
				{
					strcpy(colourful_orders[current_line].label, item);
					max_length1_FIX = max(strlen(item), max_length1_FIX);
				}
				else current_item = 1;
			}
			if (current_item == 1)
			{
				strcpy(colourful_orders[current_line].command, item);
				max_length2_FIX = max(strlen(item), max_length2_FIX);
			}
			if (current_item == 2) strcpy(colourful_orders[current_line].arg, item);
			else if (current_item >= 2) strcat(colourful_orders[current_line].arg, item);

			current_item++;
			item = strtok(NULL, " 	\n");
		}
		current_line++;
	}
	return;
} //FixImportedTxt

//############      GŁÓWNE FUNKCJE INTERPRETERA PSEUDOASSEMBLERA     #############

void MainLoop(void)
{
	int current_line = memory_lines;
	char copy_of_line[MAX_LINE];
	next_command_reg = memory_lines;
	while (current_line < memory_lines + order_lines)
	{
		strcpy(copy_of_line, orders[current_line].asm);
		ExecuteCommand(copy_of_line, current_line);
		if (RULE_linebyline == 1 && current_line >= memory_lines + input_lines) PresentProgramStatus(current_line);
		current_line = next_command_reg;
	}
	PresentProgramStatus(LAST_LINE);
	return;
} //MainLoop

void ExecuteCommand(char asm_line[MAX_LINE], int current_line)
{
	int current_item = 0, type_of_item = 0, type_of_command = 0, loc1 = 0, loc2 = 0, element_size = 0, first_location = 0;
	char* item, * command = DEFAULT_STR, * arg1 = DEFAULT_STR, * arg2 = DEFAULT_STR, * arg3 = DEFAULT_STR;
	item = strtok(asm_line, " ,()*<>	\n");
	while (item != NULL)
	{
		type_of_item = ItemType(item);
		if (type_of_item >= 1 && current_item == 0) current_item = 1;

		if (current_item == 1) command = item;
		else if (current_item == 2) arg1 = item;
		else if (current_item == 3) arg2 = item;
		else if (current_item == 4) arg3 = item;
		item = strtok(NULL, " ,()*<>	\n");
		current_item++;
	}

	type_of_command = ItemType(command);
	if (3 <= type_of_command && type_of_command <= 4) //Wczytanie i wypisanie
	{
		loc1 = FindWord(arg1, &element_size);

		if (loc1 == NOT_FOUND) ShowError(current_line, 3, "Argument 1. zawiera nieznane słowo");

		if (strcmp(arg2, DEFAULT_STR) == 0) loc2 = 1;
		else loc2 = atoi(arg2); //READ <etykieta>(ile)

		if (loc1 + element_size >= memory_size) ShowError(current_line, 4, "Argument odwołuje się do adresu spoza przestrzeni pamięci.");
		if (loc2 > element_size) ShowWarning("Polecenie odwołuje się do elementów \n nienależących do danej zmiennej.");

		if (type_of_command == 3) do_READ(loc1, loc2, arg1);
		else if (type_of_command == 4) do_WRITE(loc1, loc2, arg1);
	}
	else if (5 <= type_of_command && type_of_command <= 10) //Rejestr-Rejestr
	{
		loc1 = atoi(arg1);
		loc2 = atoi(arg2);

		if (loc1 < 0 || loc1 >= REG_NUM) ShowError(current_line, 3, "Argument 1. nie jest poprawnym rejestrem.");
		if (loc2 < 0 || loc2 >= REG_NUM) ShowError(current_line, 4, "Argument 2. nie jest poprawnym rejestrem.");

		if (type_of_command == 5) do_A(loc1, loc2);
		else if (type_of_command == 6) do_S(loc1, loc2);
		else if (type_of_command == 7) do_M(loc1, loc2);
		else if (type_of_command == 8) do_D(loc1, loc2);
		else if (type_of_command == 9) do_C(loc1, loc2);
		else if (type_of_command == 10) do_L(loc1, loc2);
	}
	else if (11 <= type_of_command && type_of_command <= 18) //Rejestr-Pamięć
	{
		loc1 = atoi(arg1);

		if (loc1 < 0 || loc1 >= REG_NUM) ShowError(current_line, 3, "Argument 1. nie jest poprawnym rejestrem.");

		if (IsNumber(arg2) == 1)
		{
			loc2 = atoi(arg2);
			if (strcmp(arg3, DEFAULT_STR) == 0) loc2 = (loc2 / INT_SIZE) + REG_NUM; // Adres komórki
			else loc2 = (loc2 / INT_SIZE) + REG_NUM + (memory[atoi(arg3)] / INT_SIZE); //Adres komórki + zawartość rejestru	
		}
		else
		{
			loc2 = FindWord(arg2, &element_size);
			first_location = loc2;

			if (loc2 == NOT_FOUND) ShowError(current_line, 4, "Argument 2. zawiera nieznane słowo");

			if (strcmp(arg3, DEFAULT_STR) == 0) loc2 += 0; //Adres komórki (z etykiety)
			else loc2 += memory[atoi(arg3)] / INT_SIZE; //Adres komórki (z etykiety) + zawartość rejestru

			if (loc2 >= element_size + first_location) ShowWarning("Polecenie odwołuje się do elementów \nnienależących do danej zmiennej.");
		}

		if (loc2 < 0 || loc2 >= memory_size) ShowError(current_line, 4, "Argument 2. odwołuje się do adresu spoza przestrzeni pamięci.");

		touched_element_loc = loc2;

		if (type_of_command == 11) do_A(loc1, loc2);
		else if (type_of_command == 12) do_S(loc1, loc2);
		else if (type_of_command == 13) do_M(loc1, loc2);
		else if (type_of_command == 14) do_D(loc1, loc2);
		else if (type_of_command == 15) do_C(loc1, loc2);
		else if (type_of_command == 16) do_L(loc1, loc2);
		else if (type_of_command == 17) do_LA(loc1, loc2);
		else if (type_of_command == 18) do_ST(loc1, loc2);
	}
	else if (19 <= type_of_command && type_of_command <= 22) //Instrukcja skoku
	{
		loc1 = FindLabel(arg1);

		if (loc1 == NOT_FOUND) ShowError(current_line, 3, "Argument 1. zawiera nieznaną etykietę.");

		if (type_of_command == 19) do_J(loc1);
		else if (type_of_command == 20) do_JZ(loc1);
		else if (type_of_command == 21) do_JP(loc1);
		else if (type_of_command == 22) do_JN(loc1);
	}
	return;
} //ExecuteCommand

//######      FUNKCJE ODPOWIEDZIALNE ZA GRAFICZNĄ PREZENTACJĘ WYNIKÓW      #######
//######                ORAZ ZA KOMUNIKACJĘ Z UŻYTKOWNIKIEM                #######
void UserPreDialog(void)
{
	char user_decision;
	printf("Czy chcesz obserwować działanie programu linijka po linijce? (\x1b[94mT/N\x1b[0m): ");
	printf("\x1b[94m");
	scanf_s(" %c", &user_decision, sizeof(user_decision));
	printf("\x1b[0m");
	if (user_decision == 'T' || user_decision == 't') RULE_linebyline = 1;

	printf("\nCzy wyswietlać całą zawartość pamięci? (\x1b[94mT/N\x1b[0m): ");
	printf("\x1b[94m");
	scanf_s(" %c", &user_decision, sizeof(user_decision));
	printf("\x1b[0m");
	if (user_decision == 'T' || user_decision == 't') RULE_showallmemory = 1;
	system("cls");
	return;
} //UserPreDialog


void PresentProgramStatus(int current_line)
{
	int i, j;
	int column1_length = 0, column2_length = 0, line_length = 0;
	int column1_lines = 2, column2_lines = 2, column3_lines = 2, all_console_lines;
	int first_line = 0;
	char str_buff[MAX_WORD];

	//NAGŁÓWEK
	RestoreCursorPosition();
	if (current_line >= 0) printf("\x1b[1;4mSTAN PROGRAMU PO WYKONANIU KOMENDY %d.\x1b[0m\n", current_line + 1);
	else printf("\x1b[1;4mKOŃCOWY STAN PROGRAMU\x1b[0m                                \n");
	printf("\n\n\n");
	printf("\x1b[s");

	//WYŚWIETLANIE KODU PROGRAMU - KOLUMNA 1
	printf("\x1b[1;4mKOD PROGRAMU\x1b[0m\n");

	if (RULE_linebyline == 0 && current_line == LAST_LINE)
		for (i = 0; i < memory_size; i++) prev_memory[i] = memory[i];

	for (i = 0; i < memory_lines + order_lines; i++)
	{
		if (i == current_line)
		{
			printf("\x1b[36;107m%3d. ", i + 1);
			for (j = 0; j < max_length1_FIX - strlen(colourful_orders[i].label); j++) printf(" ");
			printf("%s", colourful_orders[i].label);
			printf("  %s ", colourful_orders[i].command);
			for (j = 0; j < max_length2_FIX - strlen(colourful_orders[i].command); j++) printf(" ");
			printf(" %s\x1b[0m\n", colourful_orders[i].arg);
		}
		else
		{
			printf("\x1b[94m%3d.\x1b[0m ", i + 1);
			for (j = 0; j < max_length1_FIX - strlen(colourful_orders[i].label); j++) printf(" ");
			printf("\x1b[91m%s\x1b[0m", colourful_orders[i].label);
			printf("  \x1b[96m%s\x1b[0m ", colourful_orders[i].command);
			for (j = 0; j < max_length2_FIX - strlen(colourful_orders[i].command); j++) printf(" ");
			printf(" %s\n", colourful_orders[i].arg);
		}
		column1_length = max(column1_length, strlen(orders[i].asm));
	}
	column1_length += COLUMN_MARGIN;
	column1_lines += memory_lines + order_lines;
	printf("\n");
	RestoreCursorPosition();

	//WYŚWIETLANIE REJESTRÓW, ZMIENNYCH ORAZ SŁÓW/ETYKIET - KOLUMNA 2
	printf("\x1b[%dC\x1b[1;4mREJESTRY\x1b[0m\n", column1_length);
	for (i = 0; i < REG_NUM; i++)
	{
		if (memory[i] != -INF)
		{
			if (memory[i] != prev_memory[i])
			{
				printf("\x1b[36;107m");
				printf("\x1b[%dC [ %2d ] = [ %11d ]\n", column1_length, i, memory[i]);
				printf("\x1b[0m");
			}
			else printf("\x1b[%dC [ \x1b[94m%2d\x1b[0m ] = [ %11d ]\n", column1_length, i, memory[i]);
		}
		else printf("\x1b[%dC [ \x1b[94m%2d\x1b[0m ] = [           ~ ]\n", column1_length, i);
	}
	column2_lines += REG_NUM + 1;

	printf("\x1b[%dC [ \x1b[94mRS\x1b[0m ] = [ %11s ]\n", column1_length, last_result_bit);
	column2_lines += 1;

	printf("\n\x1b[%dC\x1b[1;4mZMIENNE\x1b[0m\n", column1_length);
	for (i = 0; i < word_progress; i++)
	{
		line_length = 0;
		first_line = 1;
		printf("\x1b[%dC [ \x1b[91m%20s\x1b[0m ] = ", column1_length, words[i].key);
		if (words[i].size == 1)
		{
			if (memory[words[i].location] != -INF)
			{
				if (memory[words[i].location] != prev_memory[words[i].location]) printf("\x1b[36;107m");
				printf("[ %11d ]\n", memory[words[i].location]);
				if (memory[words[i].location] != prev_memory[words[i].location]) printf("\x1b[0m");
			}
			else printf("[           ~ ]\n");
			line_length += INT_LENGTH;
		}
		else
		{
			printf("[ ");
			for (int j = 0; j < words[i].size; j++)
			{
				if (memory[words[i].location + j] != -INF)
				{
					if (memory[words[i].location + j] != prev_memory[words[i].location + j]) printf("\x1b[36;107m");
					printf("%d", memory[words[i].location + j]);
					if (memory[words[i].location + j] != prev_memory[words[i].location + j]) printf("\x1b[0m");
					line_length += strlen(itoa(memory[words[i].location + j], str_buff, 10));
				}
				else
				{
					printf("~");
					line_length++;
				}
				line_length += 2; // 2 = przecinek i spacja

				if (line_length >= MAX_LINE + EPSILON)
				{
					printf("]");
					column2_length = max(column2_length, line_length);
					first_line = 0;
					line_length = 0;
					printf("\n\x1b[%dC [                      ] = [ ", column1_length);
					column2_lines++;
				}
				else if (j != words[i].size - 1) printf(", ");
			}
			column2_length = max(column2_length, line_length);
			printf(" ]\n");
		}
	}
	column2_lines += word_progress + 2; //2 = tytuł i linia odstępu

	printf("\n\x1b[%dC\x1b[1;4mSŁOWA\x1b[0m\n", column1_length);
	for (i = 0; i < memory_lines; i++)
		if (words[i].active)
		{
			printf("\x1b[%dC [ \x1b[91m%20s\x1b[0m ] = [ %3d ]\n", column1_length, words[i].key, 4 * words[i].location - 4 * REG_NUM);
			column2_lines++;
		}
	column2_lines += 2; //2 = tytuł i linia odstępu

	printf("\n\x1b[%dC\x1b[1;4mETYKIETY\x1b[0m\n", column1_length);
	for (i = 0; i < order_lines; i++)
		if (labels[i].active)
		{
			printf("\x1b[%dC [ \x1b[91m%20s\x1b[0m ] = [ %3d ]\n", column1_length, labels[i].key, labels[i].location + 1);
			column2_lines++;
		}
	column2_length += column1_length + MAX_WORD + COLUMN_MARGIN;
	column2_lines += 2; //2 = tytuł i linia odstępu

	//WYŚWIETLANIE ZAWARTOŚCI PAMIĘCI (opcjonalne) - KOLUMNA 3
	if (RULE_showallmemory == 1)
	{
		RestoreCursorPosition();
		printf("\x1b[%dC\x1b[1;4mPAMIĘĆ\x1b[0m\n", column2_length);
		for (i = REG_NUM; i < memory_size; i++)
		{
			if (memory[i] != -INF)
			{
				if (memory[i] != prev_memory[i])
				{
					printf("\x1b[36;107m");
					printf("\x1b[%dC [ %3d ] = [ %11d ]", column2_length, INT_SIZE * i - INT_SIZE * REG_NUM, memory[i]);
					printf("\x1b[0m");
				}
				else printf("\x1b[%dC [ \x1b[94m%3d\x1b[0m ] = [ %11d ]", column2_length, INT_SIZE * i - INT_SIZE * REG_NUM, memory[i]);
			}
			else printf("\x1b[%dC [ \x1b[94m%3d\x1b[0m ] = [           ~ ]", column2_length, INT_SIZE * i - INT_SIZE * REG_NUM);

			if (i == touched_element_loc) printf(" ◄\n");
			else printf("  \n");
		}
		column3_lines += memory_size + 2; //2 = tytuł i linia odstępu
	}
	for (i = 0; i < memory_size; i++) prev_memory[i] = memory[i];
	touched_element_loc = NOT_FOUND;

	//WYŚWIETLENIE OSTRZEŻEN I PROŚBY O POTWIERDZENIE PRZEJŚCIA DO NASTĘPNEGO KROKU
	all_console_lines = max(column1_lines, max(column2_lines, column3_lines));
	if (RULE_linebyline == 1)
	{
		RestoreCursorPosition();
		printf("\x1b[1B");
		printf("                                         \n");
		printf("                                         \n");
		printf("                                         \n");
		printf("\x1b[3A");
		if (is_warning == 1)
		{
			printf("\x1b[93m%s\x1b[0m\n", warning);
			is_warning = 0;
		}
		system("PAUSE");
	}

	if (current_line == LAST_LINE)
	{
		printf("\x1b[u");
		printf("\x1b[%dBInterpreter zakonczył działanie. Możesz teraz przeanalizować końcowy stan programu.\n", all_console_lines);
		system("PAUSE");
	}
	return;
} //PresentProgramStatus

void do_READ(int loc1, int size, char* label)
{
	int i;
	system("cls");
	if (size == 1)
	{
		printf("Podaj wartość zmiennej \x1b[91m%s\x1b[0m: ", label);
		scanf_s("%d", &memory[loc1]);
	}
	else
	{
		printf("Podaj %d pierwszych wartości z tablicy \x1b[91m%s\x1b[0m: ", size, label);
		for (i = loc1; i < loc1 + size; i++) scanf_s("%d", &memory[i]);
	}
	if (is_warning == 1)
	{
		printf("\x1b[93mOstrzeżenie: %s\x1b[0m\n", warning);
		is_warning = 0;
		system("PAUSE");
	}
	next_command_reg++;
	system("cls");
	return;
} //do_READ

void do_WRITE(int loc1, int size, char* label)
{
	next_command_reg++;
	return;
} //do_WRITE

void SetConsoleModeToColor(void)
{
	ShowWindow(GetConsoleWindow(), SW_MAXIMIZE);
	SetConsoleTitle("Interpreter pseudoassemblera");
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD dwMode = 0;
	GetConsoleMode(hOut, &dwMode);
	dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
	SetConsoleMode(hOut, dwMode);
	SetConsoleOutputCP(65001);
	return;
} //SetConsoleModeToColor

void RestoreCursorPosition(void)
{
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD pos = { 0, 0 };
	SetConsoleCursorPosition(hConsole, pos);
	return;
} //RestoreCursorPosition

void ShowCredit(void)
{
	printf("Interpreter pseudoassemblera\n");
	printf("Autor: Przemysław Dominikowski\n");
	printf("\n\n");
	return;
} //ShowCredit

void ShowError(int current_line, int problem_type, char* err_message)
{
	char* colour1 = DEFAULT_STR, * colour2 = DEFAULT_STR, * colour3 = DEFAULT_STR, * colour4 = DEFAULT_STR;
	if (problem_type == 1) colour1 = "\x1b[91m";
	else if (problem_type == 2) colour2 = "\x1b[91m";
	else if (problem_type == 3) colour3 = "\x1b[91m";
	else if (problem_type == 4) colour3 = "\x1b[91m";

	system("cls");
	printf("Wykryto krytyczny błąd pseudoassemblera w linii %d: [ ", current_line + 1);
	printf("%s%s\x1b[0m   %s%s\x1b[0m   %s%s\x1b[0m ]\n", colour1, colourful_orders[current_line].label, colour2, colourful_orders[current_line].command, colour3, colourful_orders[current_line].arg);
	printf("\x1b[91m%s\x1b[0m\n", err_message);
	exit(0);
	return;
} //ShowError

void ShowWarning(char* war_message)
{
	is_warning = 1;
	warning = war_message;
	return;
} //ShowWarning