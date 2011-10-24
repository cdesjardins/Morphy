/*
 *  morphy.h
 *  Morphy
 *
 *  Created by Martin Brazeau on 11-04-26.
 *  Copyright 2011. All rights reserved.
 *  Morphy is provided as is with no warranty of any kind.
 *
 */


#include "morphy.h"

int ntax = 9;
int outtaxa[MAX_OG_SIZE];
int intaxa[MAX_IG_SIZE];
int maxstates = 5;
bool OGdefined=false;
nodearray ingroup; 
nodearray outgroup;

void init_taxarray(int *taxarray)
{
	int i;
	
	for (i = 0; i < ntax; ++i)
		taxarray[i] = i + 1;
	
}

struct tree *alloctree(tree *newtree)
{
	int i; //Loop counter
	nodearray *newtrnodes;
	
	newtree = (tree *)calloc(sizeof(tree), 1);
	
	if (newtree == NULL) {
		printf("Error: failed to allocate new tree.\n");
		return (struct tree*) 0;
	}
	
	newtrnodes = &newtree->trnodes;
	*newtrnodes = (nodearray)calloc(sizeof(node *), (2 * ntax - 1));
	
	for (i = 0; i <= (2 * ntax - 1); ++i) {
		
		(*newtrnodes)[i] = (node *)calloc(sizeof(node), 1);
		if ((*newtrnodes)[i] == NULL) {
			printf("Error: failed to allocate new node.\n");
		}
		(*newtrnodes)[i]->tip = i + 1;
	}
	
	for (i = 0; i < ntax; ++i) {
		(*newtrnodes)[i]->tip = i + 1;
		(*newtrnodes)[i]->outedge = NULL;
		(*newtrnodes)[i]->next = NULL;
		//(*newtrnodes)[i]->tipname = NULL;
		//(*newtrnodes)[i]->apomorphies = NULL;
	}
	
	for (i = ntax; i <= (2 * ntax - 1); ++i) {
		newring((*newtrnodes)[i]);
	}
	
	return (newtree);
}

void printNewick(node *n)
{	
	/* Prints the tree in Newick format (i.e. using brackets plus commas to separate
	 * equally ranked objects. In an unrooted tree, function will be called on a 
	 * terminal node that has its start variable set to 1. */
	
	node *p;
	
	if (n->start) {
		printf("(%i,", n->tip);
		printNewick(n->outedge);
		printf(")");
		return;
	}	
	
	if (n->tip && n->outedge->next->outedge->tip && !n->outedge->next->outedge->start) {
		printf("%i", n->tip);
		return;
	}
	
	if (n->tip && !n->start) {
		printf("%i", n->tip);
		return;
	}
	
	printf("(");
	
	p = n->next;
	while (p != n) {
		printNewick(p->outedge);
		p = p->next;
		if (p != n) {
			printf(",");
		}
	}	
	printf(")");
}

void fitchdown(node *leftdesc, node *rightdesc, node *ancestor, int *stepcount)
{
    int i, j, k = 0;
	char c;
	
	/* Makes synapset of ancestor the intersection of set1 and set2
	 * if the set is non-null */
    for (i = 0; leftdesc->apomorphies[i]; ++i) {
		for (j = 0; rightdesc->apomorphies[j]; ++j) {
			if (leftdesc->apomorphies[i] == rightdesc->apomorphies[j]) {
				ancestor->apomorphies[k] = rightdesc->apomorphies[j];
				++k;
				ancestor->apomorphies[k]= '\0';
			}
		}
    }
	
	/* Makes synapset of ancestor the union of set1 and set2 if 
	 * the intersection set is null */
	if (k == 0) {
		i = 0; 
		j = 0;
		
		do {
			c = leftdesc->apomorphies[i];
			ancestor->apomorphies[i] = c;
			++i;
		} while ( leftdesc->apomorphies[i] );
		
		while (rightdesc->apomorphies[j] ){
			ancestor->apomorphies[i + j] = rightdesc->apomorphies[j];
			++j;
		} 
		
		ancestor->apomorphies[i + j]= '\0';
		
		*stepcount = *stepcount + 1;
	} 
}

void treelen(node *n, int *stepcount)
{	
	node *p;
	
	if (n->tip) {
		return;
	}
	
	p = n->next;
	while (p != n) {
		treelen(p->outedge, stepcount);
		printf("tip: %i; apomorphy: %s\n", p->outedge->tip, p->outedge->apomorphies);
		p = p->next;
	}
	
	fitchdown(n->next->outedge, n->next->next->outedge, n, stepcount);
	printf("treelen returning\n");
}

void nacount(node *n, int *stepcount)
{	
	/* Will eventually become a function analogous to treelen but that checks to see if a
	 * non-applicable score is used. In which case, it will skip the call to fitchdown, and 
	 * 'wait' to see if the next outgroup is similarly sharing a non-applicable score before 
	 * before generating the synapset for the node. */
	
	node *p;
	
	if (n->tip) {
		return;
	}
	
	p = n->next;
	while (p != n) {
		nacount(p->outedge, stepcount);
		printf("tip: %i; apomorphy: %s\n", p->outedge->tip, p->outedge->apomorphies);
		p = p->next;
	}
	
	fitchdown(n->next->outedge, n->next->next->outedge, n, stepcount);
	printf("treelen returning\n");
}

void newring(node *r1)
{
	/* Generates a new internal node composed of a ring of node structures
	 * joined in a unidirectional manner by their next pointers. Used by any
	 * function that might dynamically add branches at run-time. */
	
	node *r2, *r3;
	
	r2 = (node *) malloc(sizeof(node));
	if (r2 == NULL) {
		printf("failed to allocate memory for internal node r2\n");
	}
	r3 = (node *) malloc(sizeof(node));
	if (r2 == NULL) {
		printf("failed to allocate memory for internal node r3\n");
	}
	
	r1->next = r2;
	r2->next = r3;
	r3->next = r1;
	
	r1->tip = r2->tip = r3->tip = 0;
	r1->index = r2->index = r3->index = 0;
	r1->start = r2->start = r3->start = false;
	r1->dummy = r2->dummy = r3->dummy = false;
	
	r1->apomorphies = r2->apomorphies = r3->apomorphies = (statearray) calloc(sizeof(char), maxstates);
}



void addBranch(node *desc, node *ancest, tree *newtips, int *taxon)
{	
	/* Adds a sister terminal to an existing terminal node in the tree.
	 * Receives pointers of the ancestor and descendant of a branch, pointers to a tree struct
	 * containing an array of nodes and a pointer to an integer which is used to draw the 
	 * correct pointer for the insertion. Mostly useless, but served as an early test for
	 * my understanding of how to dynamically add branches to the tree. */
	
	node *newtip, *newn;
	
	newtips->trnodes[*taxon - 6] = (node *) malloc(sizeof(node));
	if (newtips->trnodes[*taxon - 6] == NULL) {
		printf("malloc failed in addBranch newtips->trnodes[*taxon - 6]\n");
	}
	newtips->trnodes[*taxon] = (node *) malloc(sizeof(node));
	if (newtips->trnodes[*taxon] == NULL) {
		printf("malloc failed in addBranch newtips->trnodes[*taxon]\n");
	}
	
	newtip = newtips->trnodes[*taxon - 6];
	newtip->apomorphies = malloc(3 * sizeof(char));
	if (newtip->apomorphies == NULL) {
		printf("malloc failed in addBranch newtip->apomorphies\n");
	}
	
	newn = newtips->trnodes[*taxon];
	
	newring(newn);
	
	ancest->outedge = newn;
	newn->outedge = ancest;
	
	newn->next->outedge = desc;
	desc->outedge = newn->next;
	
	newn->next->next->outedge = newtip;
	newtip->outedge = newn->next->next;
	
	newtip->tip = *taxon;
	*taxon = *taxon + 1;
}

void addTip(node *n, tree *newtips, int *taxon)
{
	node *p;
	
	if (n->tip) {
		addBranch(n, n->outedge, newtips, taxon);
		return;
	}
	
	p = n->next;
	while (p != n) {
		addTip(p->outedge, newtips, taxon);
		p = p->next;
	}
}

void applyData(tree *currenttree, char **tipdata, int ntaxa, int *start)
{	
	int i;
	
	for (i = 0; i < ntaxa; ++i) {
		currenttree->trnodes[i]->apomorphies = tipdata[i + *start];
	}
	
	*start = *start + i;	// Maybe i - 1
}


void growcopy(node *templ, node *target, tree *newtree, int *iter)
{	
	/* growcopy traverses the original tree, adding branches to newtree in preorder.
	 * templ is a template node in the original tree, and target is the node in the
	 * new tree to which a new branch and node are to be attached */
	
	node *p;
	
	if (templ->tip) {
		newtree->trnodes[templ->tip - 1]->outedge = target;
		target->outedge = newtree->trnodes[templ->tip - 1];;
		newtree->trnodes[templ->tip - 1]->tip = templ->tip;
		newtree->trnodes[templ->tip - 1]->visited = templ->visited;
		newtree->trnodes[templ->tip - 1]->start = templ->start;
		
		return;
	}
	
	if (templ->outedge != NULL && !templ->outedge->start) {
		
		node *newr1;
		
		newtree->trnodes[ntax + *iter] = (node *) malloc(sizeof(node));
		newr1 = newtree->trnodes[ntax + *iter];
		newr1->visited = templ->visited;
		newring(newr1);
		
		newr1->outedge = target;
		target->outedge = newr1;
		
		target = newr1->next;
		
		*iter = *iter + 1;
		
	} else {
		target = target->next;
	}
	
	p = templ->next;
	while (p != templ) {
		growcopy(p->outedge, target, newtree, iter);
		p = p->next;
		target = target->next;
	}
}


void copytree(tree *origtree, tree *newtree, long long int *counter)
{
	int i, totalnodes, iteration = 1;
	int *iter = &iteration;
	node *base1;
	
	if (origtree->root) {
		totalnodes = 2 * ntax - 1;
	} else {
		totalnodes = 2 * ntax - 2;
	}
	
	// Allocate memory for the new tree and the new node array.
	
	if (!newtree) {	
		newtree = (tree *) malloc(sizeof(tree));
		if (newtree == NULL) {
			printf("failed to allocate memory for new tree in copytree\n");
		}
		newtree->trnodes = (nodearray) malloc(totalnodes * sizeof(node));
		if (newtree->trnodes == NULL) {
			printf("failed to allocate memory for nodes in copytree\n");
		}
	}
	
	for (i = 0; i < totalnodes; ++i) {
		newtree->trnodes[i] = (node *) malloc(sizeof(node));
		if (newtree->trnodes[i] == NULL) {
			printf("failed to allocate memory for node %i in copytree\n", i);
		}
		if (i >= ntax) {
			newtree->trnodes[i]->tip = 0;
		}
	}
	
	if (origtree->root == NULL) {
		
		/**** If the tree is unrooted ****/
		/* Generates the starting tip node and its corresponding internal
		 * node first. */
		
		//printf("unrooted copying\n");
		newtree->root == NULL;
		origtree->trnodes[0]->start = true;
		newtree->trnodes[0]->start = origtree->trnodes[0]->start;
		newtree->trnodes[0]->tip = origtree->trnodes[0]->tip;
		newring(newtree->trnodes[ntax + 1]);
		newtree->trnodes[0]->outedge = newtree->trnodes[ntax + 1];
		newtree->trnodes[ntax + 1]->outedge = newtree->trnodes[0];
		growcopy(origtree->trnodes[0]->outedge, newtree->trnodes[ntax + 1], newtree, iter);
		
		/*printf("copiedtree: ");
		 printNewick(newtree->trnodes[0]);
		 printf("\n");*/
	} else {
		
		/**** If the tree is rooted ****/
		/* Simply generates the root ring first and begins from there */
		
		base1 = newtree->trnodes[ntax];
		newring(base1);
		newtree->root = newtree->trnodes[ntax];
		growcopy(origtree->root, newtree->root, newtree, iter);
		
		printf("copiedtree: ");
		printNewick(newtree->root);
		printf("\n");
	}
	
	*counter++;
}

void roottree(tree *toroot, int root)
{
	
	/* root a tree with this function eventually*/
	
}

void deroot(tree *rootedtree)
{
	node *proot, *leftdesc, *rightdesc;
	
	proot = rootedtree->root;
	leftdesc = proot->next->outedge;
	rightdesc = proot->next->next->outedge;
	
	leftdesc->outedge = rightdesc;
	rightdesc->outedge = leftdesc;
	
	rootedtree->root = NULL;
	rootedtree->trnodes[0]->start = true; // Could, in the future, be user-defined.
	
}

void detree(node *n)
{
	node *p;
	
	if (n->tip && !n->start)
		return;
	
	//if (n->apomorphies && !n->tip) {
	//	free(n->apomorphies);
	//}
	
	if (n->start) {
		detree(n->outedge);
		return;
	}	
	
	p = n->next;
	while (p != n) {
		detree(p->outedge);
		p = p->next;
	}
	
	free(n->apomorphies);
}

void denode(nodearray trnptr)
{
	int i;
	
	node *p, *q;
	
	for (i = 0; i < ntax; ++i) {
		free(trnptr[i]);
		//trnptr[i] = NULL;
	}
	
	for (i = ntax; i <= (2 * ntax - 1); ++i) {
		if ( (trnptr[i] != NULL) && (trnptr[i]->next != NULL)) {
			p = trnptr[i]->next;
			do {
				q = p->next;
				free(p);
				p = q;
			} while (p != trnptr[i]);
			free(trnptr[i]->apomorphies);
			free(trnptr[i]);
			//trnptr[i] = NULL;
		}
	}
	
	free(trnptr);
	//trnptr = NULL;
}


int main (void) 
{
	int i; //Loop counter
	int c;
	
	//allunrooted();
	
	/*generate a random tree*/
	
	do {
		printf("Enter c to continue: \n");
		c = getchar();
	} while (c != 'c');
	c = getchar();
	
	
	tree **randtrees;
	randtrees = (tree **) malloc(150000 * sizeof(tree*));
	if (randtrees == NULL) {
		printf("Error in main(): failed malloc for randtrees\n");
		return 1;
	}
	
	for (i = 0; i < 150000; ++i) {
		randtrees[i] = randrooted(randtrees[i]);
		//printNewick(randtrees[i]->root);
		//printf(";\n");
	}
	
	do {
		printf("Enter c to continue: \n");
		c = getchar();
	} while (c != 'c');
	c = getchar();
	
	for (i = 0; i < 150000; ++i) {
		denode(randtrees[i]->trnodes);
		free(randtrees[i]);
	}
	
	free(randtrees);
	randtrees = NULL;
	
	do {
		printf("Enter c to continue: \n");
		c = getchar();
	} while (c != 'c');
	c = getchar();
	
	printf("One more time\n");
	
	randtrees = (tree **) malloc(150000 * sizeof(tree*));
	if (randtrees == NULL) {
		printf("Error in main(): failed malloc for randtrees\n");
		return 1;
	}
	
	for (i = 0; i < 150000; ++i) {
		randtrees[i] = randrooted(randtrees[i]);
		//printNewick(randtrees[i]->root);
		//printf(";\n");
	}
		
	do {
		printf("Enter c to continue: \n");
		c = getchar();
	} while (c != 'c');
	c = getchar();
		
	for (i = 0; i < 150000; ++i) {
		denode(randtrees[i]->trnodes);
		free(randtrees[i]);
	}
	
	do {
		printf("Enter c to continue: \n");
		c = getchar();
	} while (c != 'c');
	c = getchar();
	
	free(randtrees);
	randtrees = NULL;

	defOutgroup();
	
	//printf("%i\n", outtaxa[i]);
	
	return 0;
	
}
