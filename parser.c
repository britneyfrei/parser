# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include "cards.h"

int compare(const void *a, const void *b);
void parse(struct card **cards, int i, char *ptr);
const char *rarities[] = { "common", "uncommon", "rare", "mythic" };

int main(int argc, char *argv[]) {
	// check if second argument is inputted on the command line
	if (argv[1] == NULL) {
		printf("Error\n");
		return 1;
	}
	// open given file to read
	// input = file stream
	FILE *input = fopen(argv[1], "r");
	// check if the given file exists
	// if not, output to stderr and return 1
	if (input == NULL) {
		fprintf(stderr, "%s%s%s", "./parser: cannot open(\"", argv[1], 
			"\"): No such file or directory\n");
		return 1;
	}
	// allocate initial memory to cards
	struct card **cards = malloc(sizeof(struct card*));
	// entries_size = i + 1, used as size of new allocated memory
	int entries_size = 0;
	// i = index/line of the file
	int i = 0;
	// initialize size and buf for getline()
	size_t size = 0;
	// buf is where getline() stores the line
	char *buf = NULL;
	// initialize dup
	int dup = 0;
	// length = length of line
	ssize_t length;
	// skip header line
	getline(&buf, &size, input);
	// loop until reach the end of the file
	while ((length = getline(&buf, &size, input)) != -1) {
		// increment entries_size at the beginning so it starts at 1
		entries_size++;
		// reallocate memory that is the size of card * entries_size to cards
		cards = realloc(cards, sizeof(struct card) * entries_size);
		// allocate memory that is the size of card
		cards[i] = malloc(sizeof(struct card));
		// store characters of buf in array
		char *array = strdup(buf);
		// make a copy of array to make changes to
		char *stringp = array;
		// save the starting address to free later
		cards[i]->start = array;
		// delimit using comma then convert string to an int to get id field
		cards[i]->id = atoi(strsep(&stringp, ","));
		// advance stringp to first char of name
		stringp++;
		// delimit using quote to get name field
		cards[i]->name = strsep(&stringp, "\"");
		// set dup to 0
		dup = 0;
		// iterate through the previous cards
		for (int j = 0; j < i; j++) {
			// check if the current name matches any of the previous names
			if (strcmp(cards[i]->name, cards[j]->name) == 0) {
				// if so, check if the current id is greater than the previous
				if (cards[i]->id > cards[j]->id) {
					//if so, free the card at position j
					free(cards[j]->start);
					free(cards[j]);
					// parse the rest of the string
					parse(cards, i, stringp);
					for (; j < i; j++) {
						cards[j] = cards[j+1];
					}
				}
				else {
					//otherwise, free the card at position i
					free(cards[i]->start);
					free(cards[i]);
				}
				
				// if a duplicate was found, set dup to 1, then exit loop
				dup = 1;
				break;
			}
		}
		// check if the card was a duplicate
		if (dup != 1) {
			// if not, then parse the rest of the line and store in cards struct
			parse(cards, i, stringp);
			// increment i
			i++;
		}
		// if so, skip the parse, don't increment i, and move on to the next line
	}
	// if the last line in the file was a duplicate, set it to null
	if (dup == 1) {
		cards[i] = NULL;
	}
	// call qsort to sort alphabetically
	qsort(cards, i, sizeof(struct card *), compare);
	// iterate through all of the cards
	for (int j = 0; j < i; j++) {
		// width = length of name field
		int width = strlen(cards[j]->name);
		// print each card with correct formatting
		printf("%-*s%*s", width, cards[j]->name, 52 - width, cards[j]->cost);
		printf("\n%-44s%8s\n", cards[j]->type, rarities[cards[j]->rarity]);
		for (int n = 0; n < 52; n++) {
			printf("-");
		}
		printf("\n%s\n", cards[j]->text);
		for (int n = 0; n < 52; n++) {
			printf("-");
		}
		printf("\n%52s\n\n", cards[j]->stats);
	}
	// free each array's memory allocated using strdup()
	for (int j = 0; j < i; j++) {
		free(cards[j]->start);
	}
	// free each card's memory allocated using malloc()
	for (int j = 0; j < i; j++) {
		free(cards[j]);
	}
	// free buf memory allocated using getline()
	free(buf);
	// free cards memory allocated using malloc()
	free(cards);
	// close the file
	fclose(input);
	return 0;
}

int compare(const void *a, const void *b) {
	return strcmp((*(struct card **) a)->name,
                      (*(struct card **) b)->name);
}

void parse(struct card **cards, int i, char *ptr) {
	// ptr = stringp
	// advance ptr
	ptr++;
	// if the first char of pointer is a comma, there was 2 commas in a row
	if (ptr[0] == ',') {
		// so the cost field is set to an empty space
		cards[i]->cost = " ";
	}
	// otherwise, advance ptr to first char of cost
	else {
		ptr++;
		// delimit using a quote to get cost field
		cards[i]->cost = strsep(&ptr, "\"");
	}
	// advance ptr to first char of converted_cost
	ptr++;
	// delimit using a comma then convert string to int to get converted_cost
	cards[i]->converted_cost = atoi(strsep(&ptr, ","));
	// advance ptr to first char of type
	ptr++;
	// delimit using a quote to get type field
	cards[i]->type = strsep(&ptr, "\"");
	// advance ptr to first char of text
	ptr++;
	// make text = ptr
	// starts at the beginning of the text field
	// ends at end of line, including the stats and rarity fields
	cards[i]->text = ptr;
	// advance ptr to the end of the line
	ptr += strlen(ptr);
	// c = number of commas found
	int c = 0;
	// n = length of text field
	int n;
	// loop until 2 commas are found
	for (n = strlen(cards[i]->text); c < 2; n--) {
		// decrement ptr
		ptr--;
		// check if the first char of ptr is a comma
		if (ptr[0] == ',') {
			// if so, increment c
			c++;
		}
	}
	// n = index of comma at the end of actual text field
	// check if the character before n is a quote
	if (cards[i]->text[n - 1] == '"') {
		// if so, null terminate at that char
		cards[i]->text[n - 1] = 0;
	}
	// otherwise, there is 2 commas in a row
	else {
		// so make text field = null
		cards[i]->text[n] = 0;
	}
	// j = index
	int j = 0;
	// loop through text field searching for 2 double quotes
	for (n = 0; n < strlen(cards[i]->text); n++) {
		// check if current char is not a quote
		if (cards[i]->text[n] != '"') {
			// if so, assign char to position j, then increment j
			cards[i]->text[j++] = cards[i]->text[n];
		}
		// check if current char is a quote and the next char is a quote
		else if (cards[i]->text[n] == '"' && cards[i]->text[n+1] == '"') {
			// if so, assign quote to position j, then increment j
			cards[i]->text[j++] = '"';
			// also increment n to skip the second quote
			n++;
		}
	}
	// null terminate at position j
	cards[i]->text[j] = 0;
	// c = index
	c = 0;
	// loop through text field searching for newlines
	for (int n = 0; n < j; n++) {
		// check if current char is not a backslash
		if (cards[i]->text[n] != '\\') {
			// if so, assign char to position c, then increment c
			cards[i]->text[c++] = cards[i]->text[n];
		}
		// check if current char is a backslash and the next char is an 'n'
		else if (cards[i]->text[n] == '\\' && cards[i]->text[n+1] == 'n') {
			// if so, assign newline to position c, then increment c
			cards[i]->text[c++] = '\n';
			// also increment n to skip the 'n'
			n++;
		}
	}
	// null terminate at position c
	cards[i]->text[c] = 0;
	// check if text field is null
	if (cards[i]->text == NULL) {
		// if so, make it an empty space
		cards[i]->text = " ";
	}
	// advance ptr
	ptr++;
	// check if the first char of ptr is a quote
	if (ptr[0] == '"') {
		// if so, advance ptr to the first char of stats
		ptr++;
		// delimit using a quote to get stats field
		cards[i]->stats = strsep(&ptr, "\"");
	}
	// otherwise, there are 2 commas in a row
	else {
		// make stats field an empty space
		cards[i]->stats = " ";
	}
	// advance ptr to the first char of rarity field
	ptr += 2;
	// delimit using quote to get rarity string
	char *rarity = strsep(&ptr, "\"");
	// check what the rarity string is, then assign the corresponding value to rarity field
	if (strcmp(rarity, "common") == 0) {
		cards[i]->rarity = common;
	}
	else if (strcmp(rarity, "uncommon") == 0) {
		cards[i]->rarity = uncommon;
	}
	else if (strcmp(rarity, "rare") == 0) {
		cards[i]->rarity = rare;
	}
	else {
		cards[i]->rarity = mythic;
	}
	return;
}