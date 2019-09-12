#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define BUFFER_LENGTH 64
#define ERROR_MESSAGE "Error parsing instructions"
#define EMPTY_TREE_REPORT "none"

/********************************************************************************************************************************/

/*
	This enumerated type contains all the possible commands
*/
typedef enum command { ADDENT, DELENT, ADDREL, DELREL, REPORT, END } t_command;

/*
	This function checks if the string s is a command and returns the correct enumerated of the command
*/
t_command get_command(char * s) {
	if (strcmp(s,"addent") == 0)
		return ADDENT;
	if (strcmp(s,"delent") == 0)
		return DELENT;
	if (strcmp(s,"addrel") == 0)
		return ADDREL;
	if (strcmp(s,"delrel") == 0)
		return DELREL;
	if (strcmp(s,"report") == 0)
		return REPORT;
	if (strcmp(s,"end") == 0)
		return END;
	return -1;
}

/********************************************************************************************************************************/

/*
	This enumerated type contains the possible colors of a node
*/
typedef enum color { RED, BLACK } t_color;

/*
	This is the struct of the nodes of the tree
*/
typedef struct s_node {
	char * key;
	struct s_node * left;
	struct s_node * right;
	struct s_node * p;
	t_color color;
	void * extra_data;
} t_node;

/*
	This is the struct of the tree
*/
typedef struct s_tree {
	t_node * root;
	t_node * nil;
	void * extra_data;
} t_tree;

/*
	This is the reference to the list of the reusable nodes
*/
t_node * node_storage = NULL;

/*
	This function returns an initialized node
*/
t_node * get_node(t_tree * tree, char * key) {
	unsigned length;
	t_node * new;

	if (node_storage != NULL) {
		new = node_storage;
		node_storage = node_storage -> extra_data;
	} else
		new = malloc(sizeof *new);

	new -> left = tree -> nil;
	new -> right = tree -> nil;
	new -> p = tree -> nil;
	new -> color = BLACK;
	new -> extra_data = NULL;
	if (key) {
		length = strlen(key);
		new -> key = malloc((length + 1) * sizeof *(new -> key));
		strncpy(new -> key,key,length + 1);
	} else
		new -> key = NULL;

	return new;
}

/*
	This function stores a node to be reused
*/
void throw_node(t_node * node) {

	free(node -> key);
	node -> extra_data = node_storage;
	node_storage = node;

	return;
}

/*
	This function initializes the tree references
*/
void init_tree(t_tree ** tree_ref) {

	*tree_ref = malloc(sizeof **tree_ref);

	(*tree_ref) -> nil = get_node(*tree_ref,NULL);
	(*tree_ref) -> nil -> left = (*tree_ref) -> nil;
	(*tree_ref) -> nil -> right = (*tree_ref) -> nil;
	(*tree_ref) -> nil -> p = (*tree_ref) -> nil;

	(*tree_ref) -> root = (*tree_ref) -> nil;

	(*tree_ref) -> extra_data = NULL;

	return;
}

/*
	This function returns the reference to the node with the specified key
	It returns a reference to nil if the key is not found
*/
t_node * search_node(t_node * root, t_node * nil, char * key) {

	if ((root == nil) || (strncmp(key, root -> key, BUFFER_LENGTH) == 0))
		return root;
	if (strncmp(key, root -> key, BUFFER_LENGTH) < 0)
		return search_node(root -> left, nil, key);
	return search_node(root -> right, nil, key);
}

/*
	This function returns the reference to the last node of the tree
*/
t_node * tree_max(t_node * root, t_node * nil) {

	if (root -> right != nil)
		return tree_max(root -> right, nil);

	return root;
}

/*
	This function returns the reference to the first node of the tree
*/
t_node * tree_min(t_node * root, t_node * nil) {

	if (root -> left != nil)
		return tree_min(root -> left, nil);

	return root;
}

/*
	This function returns the reference to the successor of the node x
*/
t_node * tree_successor(t_node * x, t_node * nil) {
	t_node * y;

	if (x -> right != nil)
		return tree_min(x -> right, nil);
	y = x -> p;
	while ((y != nil) && (x == y -> right)) {
		x = y;
		y = y -> p;
	}

	return y;
}

/*
	This function frees all the nodes of the tree
*/
void free_tree(t_node * root, t_node * nil) {

	if (root -> left != nil)
		free_tree(root -> left, nil);

	if (root -> right != nil)
		free_tree(root -> right, nil);

	if (root != nil)
		throw_node(root);

	return;
}

/*
	This function left-rotates the x node of the tree
*/
void left_rotate(t_tree * tree, t_node * x) {
	if (x -> right == tree -> nil)
		return;

	t_node * y = x -> right;

	x -> right = y -> left;
	if (y -> left != tree -> nil)
		y -> left -> p = x;
	y -> p = x -> p;
	if (x -> p == tree -> nil)
		tree -> root = y;
	else if (x == x -> p -> left)
		x -> p -> left = y;
	else
		x -> p -> right = y;
	y -> left = x;
	x -> p = y;

	return;
}

/*
	This function right-rotates the x node of the tree
*/
void right_rotate(t_tree * tree, t_node * x) {
	if (x -> left == tree -> nil)
		return;

	t_node * y = x -> left;

	x -> left = y -> right;
	if (y -> right != tree -> nil)
		y -> right -> p = x;
	y -> p = x -> p;
	if (x -> p == tree -> nil)
		tree -> root = y;
	else if (x == x -> p -> left)
		x -> p -> left = y;
	else
		x -> p -> right = y;
	y -> right = x;
	x -> p = y;

	return;
}

/*
	This function must be called only on red nodes, as it restores the red-black conditions on the tree after an insertion
*/
void insert_fixup(t_tree * tree, t_node * z) {
	t_node * x;
	t_node * y;

	if (z == tree -> root)
		(tree -> root) -> color = BLACK;
	else {
		x = z -> p;
		if (x -> color == RED) {
			if (x == x -> p -> left) {
				y = x -> p -> right;
				if (y -> color == RED) {
					x -> color = BLACK;
					y -> color = BLACK;
					x -> p -> color = RED;
					insert_fixup(tree,x -> p);
				} else {
					if (z == x -> right) {
						z = x;
						left_rotate(tree,z);
						x = z -> p;
					}
					x -> color = BLACK;
					x -> p -> color = RED;
					right_rotate(tree,x -> p);
				} 
			} else {
				y = x -> p -> left;
				if (y -> color == RED) {
					x -> color = BLACK;
					y -> color = BLACK;
					x -> p -> color = RED;
					insert_fixup(tree,x -> p);
				} else {
					if (z == x -> left) {
						z = x;
						right_rotate(tree,z);
						x = z -> p;
					}
					x -> color = BLACK;
					x -> p -> color = RED;
					left_rotate(tree,x -> p);
				}
			}
		}
	}

	return;
}

/*
	This function adds a new node to the tree keeping it sorted
	It returns the reference to the added node
	Duplicated keys are ignored
	If increment_counter is true, when the insertion is successful, extra_data is incremented
*/
t_node * tree_insert(t_tree * tree, char * key, bool increment_counter) {
	t_node * x = tree -> root;
	t_node * y = tree -> nil;
	t_node * z;

	while (x != tree -> nil) {
		y = x;
		if (strncmp(key, x -> key, BUFFER_LENGTH) == 0)
			return x;
		if (strncmp(key, x -> key, BUFFER_LENGTH) < 0)
			x = x -> left;
		else
			x = x -> right;
	}
	z = get_node(tree,key);
	z -> p = y;
	if (y == tree -> nil)
		tree -> root = z;
	else if (strncmp(z -> key, y -> key, BUFFER_LENGTH) < 0)
		y -> left = z;
	else
		y -> right = z;
	z -> left = tree -> nil;
	z -> right = tree -> nil;
	z -> color = RED;
	insert_fixup(tree, z);

	if (increment_counter && (tree -> extra_data != NULL))
		(* (int *) tree -> extra_data)++;

	return z;
}

/*
	This function restores the red-black conditions on the tree after a deletion
*/
void delete_fixup(t_tree * tree, t_node * x) {
	t_node * w;

	if ((x -> color == RED) || (x -> p == tree -> nil))
		x -> color = BLACK;
	else if (x == x -> p -> left) {
		w = x -> p -> right;
		if (w -> color == RED) {
			w -> color = BLACK;
			x -> p -> color = RED;
			left_rotate(tree, x -> p);
			w = x -> p -> right;
		}
		if ((w -> left -> color == BLACK) && (w -> right -> color == BLACK)) {
			w -> color = RED;
			delete_fixup(tree, x -> p);
		} else {
			if (w -> right -> color == BLACK) {
				w -> left -> color = BLACK;
				w -> color = RED;
				right_rotate(tree, w);
				w = x -> p -> right;
			}
			w -> color = x -> p -> color;
			x -> p -> color = BLACK;
			w -> right -> color = BLACK;
			left_rotate(tree, x -> p);
		}
	} else {
		w = x -> p -> left;
		if (w -> color == RED) {
			w -> color = BLACK;
			x -> p -> color = RED;
			right_rotate(tree, x -> p);
			w = x -> p -> left;
		}
		if ((w -> right -> color == BLACK) && (w -> left -> color == BLACK)) {
			w -> color = RED;
			delete_fixup(tree, x -> p);
		} else {
			if (w -> left -> color == BLACK) {
				w -> right -> color = BLACK;
				w -> color = RED;
				left_rotate(tree, w);
				w = x -> p -> left;
			}
			w -> color = x -> p -> color;
			x -> p -> color = BLACK;
			w -> left -> color = BLACK;
			right_rotate(tree, x -> p);
		}
	}

	return;
}

/*
	This function removes a node from the tree keeping it sorted
	It returns the reference to the removed node
	If the key was not stored in the tree, the function returns the nil node
	If decrement_counter is true, when the deletion is successful, extra_data is decremented
*/
t_node * tree_delete(t_tree * tree, char * key, bool decrement_counter) {
	t_node * x;
	t_node * y;
	t_node * z = search_node(tree -> root, tree -> nil, key);
	void * p;

	if (z == tree -> nil)
		return z;

	if ((z -> left == tree -> nil) || (z -> right == tree -> nil))
		y = z;
	else y = tree_successor(z, tree -> nil);
	if (y -> left != tree -> nil)
		x = y -> left;
	else x = y -> right;
	x -> p = y -> p;
	if (y -> p == tree -> nil)
		tree -> root = x;
	else if (y == y -> p -> left)
		y -> p -> left = x;
	else
		y -> p -> right = x;
	if (y != z) {
		p = z -> key;
		z -> key = y -> key;
		y -> key = p;
		p = z -> extra_data;
		z -> extra_data = y -> extra_data;
		y -> extra_data = p;
	}
	if (y -> color  == BLACK)
		delete_fixup(tree, x);

	if (decrement_counter && (tree -> extra_data != NULL))
		(* (int *) tree -> extra_data)--;

	return y;
}

/********************************************************************************************************************************/

/*
	This function returns the max value of extra_data of the sender tree
*/
int get_max_val(t_node * root, t_node * nil) {
	int max_left = 0;
	int max_right = 0;
	int max_this = * (int *) ((t_tree *) root -> extra_data) -> extra_data;

	if (root -> left != nil)
		max_left = get_max_val(root -> left, nil);
	if (root -> right != nil)
		max_right = get_max_val(root -> right, nil);

	if (max_this > max_right) {
		if (max_this > max_left)
			return max_this;
		return max_left;
	}
	if (max_left > max_right)
		return max_left;
	return max_right;
}

/*
	This function updates the extra_data value of the destination tree
*/
void update_dest_tree(t_tree * tree) {

	if (tree -> root == tree -> nil)
		* (int *) tree -> extra_data = 0;
	else
		* (int *) tree -> extra_data = get_max_val(tree -> root, tree -> nil);

	return;
}

/*
	This function deletes the instances of the entity in ent_send_tree
*/
void ent_dest_tree_purge(t_node * root, t_node * nil, char * key) {
	t_node * ent_send_node;
	t_tree * ent_send_tree;

	if (root -> left != nil)
		ent_dest_tree_purge(root -> left, nil, key);

	if (root -> right != nil)
		ent_dest_tree_purge(root -> right, nil, key);

	ent_send_tree = root -> extra_data;
	ent_send_node = tree_delete(ent_send_tree, key, true);

	if (ent_send_node != ent_send_tree -> nil)
		throw_node(ent_send_node);

	return;
}

/*
	This function deletes the instances of the entity in the ent_dest_tree
	It also calls the purge for every ent_dest_tree
*/
void relation_tree_purge(t_node * root, t_node * nil, char * key) {
	t_node * ent_dest_node;
	t_tree * ent_dest_tree;
	t_tree * ent_send_tree;

	if (root -> left != nil)
		relation_tree_purge(root -> left, nil, key);

	ent_dest_tree = root -> extra_data;
	ent_dest_node = tree_delete(ent_dest_tree, key, false);

	if (ent_dest_node != ent_dest_tree -> nil) {
		ent_send_tree = ent_dest_node -> extra_data;
		free_tree(ent_send_tree -> root, ent_send_tree -> nil);
	}

	if (ent_dest_tree -> root != ent_dest_tree -> nil)
		ent_dest_tree_purge(ent_dest_tree -> root, ent_dest_tree -> nil, key);

	update_dest_tree(ent_dest_tree);

	if (root -> right != nil)
		relation_tree_purge(root -> right, nil, key);

}

/********************************************************************************************************************************/

/*
	This is the reference to the entity tree
*/
t_tree * entity_tree = NULL;

/*
	This is the reference to the relation tree
*/
t_tree * relation_tree = NULL;

/*
	This function stores a new entity in the monitored ones
*/
void function_addent(char * s) {
	tree_insert(entity_tree,s,false);

	return;
}

/*
	This function removes an existing entity from the monitored ones
	It also removes all the relations involving the entity
*/
void function_delent(char * s) {
	t_node * ent_node;

	ent_node = tree_delete(entity_tree, s, false);

	if (ent_node == entity_tree -> nil)
		return;

	throw_node(ent_node);

	if (relation_tree -> root != relation_tree -> nil)
		relation_tree_purge(relation_tree -> root, relation_tree -> nil, s);

	return;
}

/*
	This function creates a new relation between two monitored entities
	Commands with not monitored entities are ignored
*/
void function_addrel(char * s0, char * s1, char * s2) {
	t_node * rel_node;
	t_node * ent_dest_node;
	t_tree * ent_dest_tree;
	t_tree * ent_send_tree;

	if ((search_node(entity_tree -> root, entity_tree -> nil, s0) == entity_tree -> nil) ||
		(search_node(entity_tree -> root, entity_tree -> nil, s1) == entity_tree -> nil))
		return;

	rel_node = tree_insert(relation_tree, s2, false);

	ent_dest_tree = rel_node -> extra_data;

	if (ent_dest_tree == NULL) {
		ent_dest_tree = malloc(sizeof *ent_dest_tree);
		ent_dest_tree -> nil = relation_tree -> nil;
		ent_dest_tree -> root = ent_dest_tree -> nil;
		ent_dest_tree -> extra_data = malloc(sizeof *(ent_dest_tree -> extra_data));
		* (int*) ent_dest_tree -> extra_data = 0;
		rel_node -> extra_data = ent_dest_tree;
	}

	ent_dest_node = tree_insert(ent_dest_tree, s1, false);
	ent_send_tree = ent_dest_node -> extra_data;

	if (ent_send_tree == NULL) {
		ent_send_tree = malloc(sizeof *ent_send_tree);
		ent_send_tree -> nil = relation_tree -> nil;
		ent_send_tree -> root = ent_send_tree -> nil;
		ent_send_tree -> extra_data = malloc(sizeof *(ent_send_tree -> extra_data));
		* (int*) ent_send_tree -> extra_data = 0;
		ent_dest_node -> extra_data = ent_send_tree;
	}

	tree_insert(ent_send_tree, s0, true);

	if (* (int*) ent_send_tree -> extra_data > * (int*) ent_dest_tree -> extra_data)
		* (int*) ent_dest_tree -> extra_data = * (int*) ent_send_tree -> extra_data;
	
	return;
}

/*
	This function deletes a specific instance of a relation
	Commands with not existing relations are ignored
*/
void function_delrel(char * s0, char * s1, char * s2) {
	t_node * rel_node;
	t_node * ent_dest_node;
	t_node * ent_send_node;
	t_tree * ent_dest_tree;
	t_tree * ent_send_tree;

	rel_node = search_node(relation_tree -> root, relation_tree -> nil, s2);

	if (rel_node == relation_tree -> nil)
		return;

	ent_dest_tree = rel_node -> extra_data;
	ent_dest_node = search_node(ent_dest_tree -> root, ent_dest_tree -> nil, s1);

	if (ent_dest_node == ent_dest_tree -> nil)
		return;

	ent_send_tree = ent_dest_node -> extra_data;
	ent_send_node = tree_delete(ent_send_tree, s0, true);

	if (ent_send_node == ent_send_tree -> nil)
		return;

	throw_node(ent_send_node);

	if (* (int *) ent_send_tree -> extra_data == * (int *) ent_dest_tree -> extra_data - 1)
		update_dest_tree(ent_dest_tree);

	if (* (int *) ent_send_tree -> extra_data == 0) {
        ent_dest_node -> extra_data = NULL;
        free(ent_send_tree -> extra_data);
        free(ent_send_tree);
        ent_dest_node = tree_delete(ent_dest_tree, s1, false);
        throw_node(ent_dest_node);
    }

	return;
}

/********************************************************************************************************************************/

/*
	This value is used to check if the inspection of the relation tree prints something
*/
bool empty_report;

/*
	This function prints the keys of the nodes whose extra_data is equivalent to counter
*/
void report_relation_node(t_node * root, t_node * nil, int counter) {

	if (root -> left != nil)
		report_relation_node(root -> left, nil, counter);

	if (* (int *) ((t_tree *) root -> extra_data) -> extra_data == counter) {
		fputc(' ',stdout);
		fputs(root -> key, stdout);
	}

	if (root -> right != nil)
		report_relation_node(root -> right, nil, counter);

	return;
}

/*
	This function prints all the relations with at least one instance
*/
void report_relation_tree(t_node * root, t_node * nil) {
	t_tree * ent_dest_tree = root -> extra_data;
	int counter = * (int *) ent_dest_tree -> extra_data;

	if (root -> left != nil)
		report_relation_tree(root -> left, nil);

	if (* (int *) ent_dest_tree -> extra_data > 0) {

		if (!empty_report)
			fputc(' ', stdout);
		
		fputs(root -> key, stdout);
		report_relation_node(ent_dest_tree -> root, ent_dest_tree -> nil, counter);
		fprintf(stdout, " %d;", * (int *) ent_dest_tree -> extra_data);

		empty_report = false;
	}

	if (root -> right != nil)
		report_relation_tree(root -> right, nil);
	
	return;
}

/*
	This functions prints the current status of the relations graph
*/
void function_report() {

	empty_report = true;

	if (relation_tree -> root == relation_tree -> nil) {
		fputs(EMPTY_TREE_REPORT, stdout);
		fputc('\n',stdout);
		return;
	}

	report_relation_tree(relation_tree -> root, relation_tree -> nil);

	if (empty_report)
		fputs(EMPTY_TREE_REPORT, stdout);

	fputc('\n',stdout);

	return;
}

/********************************************************************************************************************************/

/*
	This function initializes all the data structures used in the program
	It is called only once, when the program starts
*/
void function_init_scenario() {
	init_tree(&entity_tree);
	init_tree(&relation_tree);

	return;
}

/*
	This function frees all the allocated space during the computation
	It is called only once, when the program ends
*/
void function_clean_scenario() {
	//TODO Free all the allocated memory

	return;
}

/********************************************************************************************************************************/


char single_scan(char * param_0) {
	int i;
	char c;

	for (i = 0; i < BUFFER_LENGTH ; i++) {
		c = fgetc(stdin);
		if ((c == ' ') || (c == '\n'))
			break;
		else if (c == EOF) {
			param_0[i] = '\0';
			return EOF;
		}
		param_0[i] = c;
	} param_0[i] = '\0';

	return 0;
}

char triple_scan(char * param_0, char * param_1, char * param_2) {
	
	single_scan(param_0);
	single_scan(param_1);
	single_scan(param_2);

	return 0;
}

int main(int argc, char const *argv[]) {
	char buffer[BUFFER_LENGTH + 1];
	char param_0[BUFFER_LENGTH + 1];
	char param_1[BUFFER_LENGTH + 1];
	char param_2[BUFFER_LENGTH + 1];

	function_init_scenario();

	while(single_scan(buffer) != EOF) {
		switch(get_command(buffer)) {
			case ADDENT: {
				single_scan(param_0);
				function_addent(param_0);
				break;
			}
			case DELENT: {
				single_scan(param_0);
				function_delent(param_0);
				break;
			}
			case ADDREL: {
				triple_scan(param_0, param_1,param_2);
				function_addrel(param_0,param_1,param_2);
				break;
			}
			case DELREL: {
				triple_scan(param_0, param_1,param_2);
				function_delrel(param_0, param_1, param_2);
				break;
			}
			case REPORT: {
				function_report();
				break;
			}
			case END: break;
			default: {
				function_clean_scenario();
				fputs(ERROR_MESSAGE,stderr);
				fputc('\n',stderr);
				return -1;
			}
		}
	}

	function_clean_scenario();

	return 0;
}
