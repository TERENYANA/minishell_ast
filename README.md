
CONSTRUCTION

00  Setup & Makefile
01  Boucle REPL
02  Signaux (prompt)
03  Environnement
04  Lexer
05  Syntaxe
06  Parseur (AST)
07  Expansion des $
08  Builtins
09  Une commande
10  Redirections
11  Pipeline |
12  Heredoc
13  Valgrind

# 🐚 minishell — Documentation complète (Phases 0–5)

> Projet : minishell (École 42) · Architecture : AST
> Statut : ✅ Phases 0–5 terminées · 🔜 Phase 6 — Parseur (AST)

---

# Phase 0 — Setup & Makefile

## Objectif

Le projet compile proprement. `make`, `clean`, `fclean`, `re` fonctionnent. Un `main` vide retourne 0.

## Concept

Avant d'écrire la moindre logique, on met en place le squelette : un Makefile robuste, la libft compilée, un `main` minimal. Chaque phase suivante ne fera qu'ajouter des fichiers `.c` à la liste `SRCS`.

## Le Makefile — points importants

```makefile
NAME       := minishell

CC         := cc
CFLAGS     := -Wall -Wextra -Werror
READLINE_PREFIX := $(shell brew --prefix readline)

INCLUDES := -I. -Ilibft -I$(READLINE_PREFIX)/include
LDLIBS   := -L$(READLINE_PREFIX)/lib -lreadline -Llibft -lft

SRCS       := \
	srcs/main.c \
	srcs/signals.c \
	srcs/env_init.c \
	srcs/env_get.c \
	srcs/env_set.c \
	srcs/free.c \
	srcs/error.c \
	srcs/debug.c \
	srcs/lexer/lexer.c \
	srcs/lexer/token.c \
	srcs/parser/syntax.c

OBJS       := $(SRCS:.c=.o)
DEPS       := $(OBJS:.o=.d)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -MMD -MP -c $< -o $@

-include $(DEPS)
```

### Pourquoi `-MMD -MP` ?

Ces flags génèrent des fichiers `.d` (dépendances). Sans eux, si tu modifies `minishell.h`, make ne recompile **pas** les `.c` qui l'incluent — et tu travailles avec des binaires obsolètes. Avec eux :

```
touch minishell.h && make   →  tous les .o se recompilent
```

### Le problème macOS / Apple Silicon

Le readline « système » de macOS est en réalité **libedit** (BSD), déguisé sous le même nom. Il lui manque des fonctions comme `rl_replace_line`. Symptôme :

```
Undefined symbols for architecture arm64:
  "_rl_replace_line", referenced from: _on_signal_prompt in signals.o
```

Solution : installer le vrai GNU Readline via brew et pointer le compilateur dessus :

```makefile
READLINE_PREFIX := $(shell brew --prefix readline)
INCLUDES := -I. -Ilibft -I$(READLINE_PREFIX)/include
LDLIBS   := -L$(READLINE_PREFIX)/lib -lreadline -Llibft -lft
```

- `-I.../include` — où le compilateur trouve `readline.h`
- `-L.../lib` — où l'éditeur de liens trouve `libreadline.dylib`

> 💡 Sur les machines Linux de l'école, `-lreadline` suffit. Pour un Makefile universel : `ifeq ($(shell uname -s),Darwin)`.

### Comment lire les erreurs de build

| Erreur | Étape | Cause | Solution |
|--------|-------|-------|----------|
| `implicit function declaration` | compilation | prototype absent | ajouter le prototype dans le `.h` |
| `Undefined symbols` | édition de liens | le corps de la fonction n'existe dans aucun `.o` | ajouter le fichier `.c` dans `SRCS` |
| `No rule to make target 'X.o'` | make | le fichier `X.c` n'existe pas sur le disque | vérifier le chemin / créer le fichier |

> 🔑 Réflexe : **chaque nouveau fichier `.c` créé → immédiatement ajouté dans `SRCS`.**

## Tests

**T0.1 — Compilation sans warnings.**
```
$ make
```

**T0.2 — Le binaire se lance et sort.**
```
$ ./minishell
$ echo $?
0
```

**T0.3 — make clean / fclean / re.**
```
$ make clean
$ make fclean
$ make re
```

**T0.4 — Pas de relink.**
```
$ make && make
make: Nothing to be done for 'all'.
```

**T0.5 — Les dépendances fonctionnent.**
```
$ touch minishell.h && make
# tous les .o se recompilent
```

---

# Phase 1 — Boucle REPL

## Objectif

Le prompt `minishell$` apparaît. readline lit une ligne. Ctrl-D quitte proprement avec `exit`.

## Concept

**REPL** = Read–Eval–Print–Loop. Chaque itération est *un univers séparé* : les objets malloc'és dans une itération **doivent mourir à la fin de celle-ci**.

> 🧠 La compétence principale du projet : le réflexe « je reçois un pointeur → je mémorise immédiatement qui va le libérer ».

## Le code — main.c

```c
#include "minishell.h"

/*
** Stub d'exécution (Phase 1).
** Deviendra le pipeline complet : tokenize → syntax → parse → run_tree.
*/
static int	run_line(char *line, t_var *env, int status)
{
	printf("got: '%s'\n", line);
	return (status);
}

/*
** Évalue l'état du shell juste après le retour de readline.
** Si un SIGINT (Ctrl-C) a été capté : reset du flag global, retour 130
** (convention Unix : 128 + numéro du signal, SIGINT = 2).
*/
static int	post_readline_status(int prev)
{
	if (g_sig == SIGINT)
	{
		g_sig = 0;
		return (130);
	}
	return (prev);
}

int	main(int argc, char **argv, char **envp)
{
	t_var	*env;
	char	*line;
	int		status;

	(void)argc;
	(void)argv;
	env = create_env(envp);
	status = 0;
	while (1)
	{
		setup_signal_handlers();
		line = readline("minishell$ ");
		if (!line)
			break ;
		status = post_readline_status(status);
		if (*line)
			add_history(line);
		status = run_line(line, env, status);
		free(line);
	}
	ft_free_env(env);
	ft_putendl_fd("exit", STDERR_FILENO);
	rl_clear_history();
	return (status);
}
```

## Explication ligne par ligne de la boucle

**`setup_signal_handlers();`** — réinstallés **à chaque itération**, car plus tard (Phase 9) on les changera pendant l'exécution des commandes (contexte enfant), puis il faudra revenir au mode prompt.

**`line = readline("minishell$ ");`** — affiche le prompt, attend l'entrée. Retourne une chaîne **malloc'ée** (sans le `\n` final). C'est nous qui devrons la libérer.

**`if (!line) break ;`** — readline retourne `NULL` sur **Ctrl-D** (EOF). C'est le signal de sortie du shell. Attention : Ctrl-D ≠ Ctrl-C (qui donne une chaîne vide `""`, pas NULL).

**`status = post_readline_status(status);`** — un seul endroit où l'on interroge `g_sig`. Si Ctrl-C a été pressé : `$?` devient 130, le flag est remis à zéro. Sinon, le status précédent est conservé.

**`if (*line) add_history(line);`** — seules les lignes **non vides** entrent dans l'historique (flèche ↑). `*line` vaut `'\0'` pour une ligne vide → condition fausse.

**`status = run_line(line, env, status);`** — le « pipeline » d'exécution. En Phase 1, un simple stub qui affiche la ligne. Il grandira à chaque phase.

**`free(line);`** — libération de la mémoire de readline. **Chaque itération.** L'oublier = fuite à chaque commande tapée.

**Après la boucle :** `ft_free_env` (la liste d'environnement vit toute la session), `exit` affiché sur stderr (comme bash), `rl_clear_history()` (l'historique interne de readline est aussi de la mémoire allouée).

## Rôle du paramètre `status`

`status` transporte le **code de retour de la dernière commande** — le futur `$?` :

1. Initialisé à 0 au démarrage.
2. Mis à jour par `post_readline_status` (Ctrl-C → 130).
3. Mis à jour par `run_line` (à partir de la Phase 9 : vrai code de la commande).
4. Retourné par `main` à la sortie → devient le `$?` du shell parent.

> ⚠️ Pas de variable globale pour `status` — le sujet n'autorise qu'**une** globale, réservée au numéro de signal. Tout le reste circule par paramètres.

## Tests

**T1.1 — Ctrl-D quitte.**
```
$ ./minishell
minishell$ [Ctrl-D]
exit
$ echo $?
0
```

**T1.2 — Lignes vides.**
```
minishell$
minishell$
minishell$        # pas de crash, pas d'affichage parasite
```

**T1.3 — Le stub répond.**
```
minishell$ hello world
got: 'hello world'
```

**T1.4 — L'historique fonctionne.** Tape une commande, puis ↑ — elle réapparaît.

**T1.5 — Valgrind propre.**
```
$ valgrind --leak-check=full ./minishell
minishell$ hello
minishell$ [Ctrl-D]
# definitely lost: 0 bytes
```

---

# Phase 2 — Signaux (contexte prompt)

## Objectif

Ctrl-C au prompt → nouvelle ligne, prompt propre, `$? = 130`. Ctrl-\ ignoré.

## Concept : `volatile sig_atomic_t`

Dans un shell, les signaux (`Ctrl+C`, `Ctrl+\`) sont des **interruptions asynchrones** : le processeur peut suspendre le code principal à n'importe quelle nanoseconde pour exécuter le handler.

`extern volatile sig_atomic_t g_sig;` est **le seul moyen sûr** de transmettre l'information « un signal est arrivé » du handler vers la boucle principale.

### Les trois composants de la construction

**`sig_atomic_t` (garantie d'atomicité).** Ce type garantit que la lecture ou l'écriture se fait en **une seule instruction processeur**. Le CPU ne peut physiquement pas être interrompu « au milieu » d'une modification.

- *Sans lui :* si un signal interrompt l'écriture d'une variable ordinaire (ex. un `long` sur 2 instructions), le handler lit une « bouillie » — moitié anciennes données, moitié nouvelles (**race condition**).
- *Avec lui :* l'opération est indivisible. Le handler voit soit strictement l'ancienne valeur, soit strictement la nouvelle.

**`volatile` (interdiction d'optimisation).** Indique au compilateur que la variable peut changer « de l'extérieur » (dans le handler). Force la lecture **directement depuis la RAM** à chaque accès, interdit le cache en registre.

- *Sans lui :* une boucle `while (g_sig == 0)` devient infinie — le compilateur « optimise » et ne relit jamais la RAM.

**`extern` (visibilité globale).** Permet d'utiliser la même variable dans tous les fichiers (`main.c`, `signals.c`, ...) via le header commun. La **définition** (sans extern, avec `= 0`) existe en un seul exemplaire dans `signals.c` :

```c
/* minishell.h — déclaration (tous les fichiers la voient) */
extern volatile sig_atomic_t	g_sig;

/* signals.c — définition (un seul exemplaire) */
volatile sig_atomic_t	g_sig = 0;
```

> ⚠️ **Contrainte du sujet 42 :** une seule variable globale, contenant **uniquement le numéro du signal**. Ni env, ni status, ni structure — rien d'autre en global.

### Pourquoi `g_sig = 0` à l'initialisation ?

Zéro signifie « aucun signal reçu ». Une valeur de départ garantie évite qu'un `if (g_sig == SIGINT)` se déclenche par accident au démarrage. C'est aussi de la lisibilité : le lecteur voit immédiatement l'état initial attendu.

## Le code — signals.c

```c
#include "minishell.h"

volatile sig_atomic_t	g_sig = 0;

static void	on_signal_prompt(int signo)
{
	g_sig = signo;
	if (signo == SIGINT)
	{
		write(STDOUT_FILENO, "\n", 1);
		rl_on_new_line();
		rl_replace_line("", 0);
		rl_redisplay();
	}
}

void	setup_signal_handlers(void)
{
	struct sigaction	sa;

	sa.sa_handler = on_signal_prompt;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;             /* sans SA_RESTART : readline doit s'interrompre */
	sigaction(SIGINT, &sa, NULL);
	signal(SIGQUIT, SIG_IGN);
}

void	ignore_signals(void)
{
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
}
```

## setup_signal_handlers — ligne par ligne

**`struct sigaction sa;`** — structure de configuration du handler, définie dans `<signal.h>` :
- `sa_handler` : pointeur vers la fonction-handler
- `sa_mask` : masque des signaux bloqués pendant l'exécution du handler
- `sa_flags` : flags modifiant le comportement

**`sa.sa_handler = on_signal_prompt;`** — quand le signal arrivera, le contrôle passera à `on_signal_prompt`.

**`sigemptyset(&sa.sa_mask);`** — masque vide : aucun signal supplémentaire bloqué pendant le handler.

**`sa.sa_flags = 0;`** — **le point crucial.** On n'active PAS `SA_RESTART`. Ce flag relancerait automatiquement les appels système interrompus. Or nous voulons l'inverse : que `readline` soit **interrompue** par Ctrl-C et rende la main à notre code. Avec `SA_RESTART`, readline continuerait d'attendre comme si de rien n'était.

**`sigaction(SIGINT, &sa, NULL);`** — installation : SIGINT (Ctrl-C) → notre handler. Le 3e argument (`NULL`) pourrait recevoir l'ancien handler — inutile ici.

**`signal(SIGQUIT, SIG_IGN);`** — Ctrl-\ est **ignoré** au prompt (comme bash). `SIG_IGN` est la constante « ignorer ».

## Le handler — ligne par ligne

```c
static void	on_signal_prompt(int signo)
{
	g_sig = signo;                        /* mémoriser le numéro */
	if (signo == SIGINT)
	{
		write(STDOUT_FILENO, "\n", 1);    /* passer à la ligne */
		rl_on_new_line();                 /* dire à readline : nouvelle ligne */
		rl_replace_line("", 0);           /* vider le buffer de saisie */
		rl_redisplay();                   /* redessiner le prompt propre */
	}
}
```

Le paramètre `signo` est fourni par le noyau : c'est le numéro du signal reçu (SIGINT = 2). Les trois fonctions `rl_*` sont l'API de GNU Readline pour manipuler son état interne depuis un handler.

> 🔑 Effet visible : l'utilisateur tape du texte, presse Ctrl-C → le texte disparaît, un prompt neuf apparaît sur une nouvelle ligne. Exactement comme bash.

## Carte des trois contextes de signaux (pour la suite)

| Contexte | SIGINT | SIGQUIT | Phase |
|----------|--------|---------|-------|
| **prompt** | nouvelle ligne + redraw | ignoré | 2 ✅ |
| **attente d'un enfant** | ignoré par le parent (l'enfant le reçoit) | ignoré par le parent | 9 |
| **heredoc** | `close(stdin)` pour interrompre readline | ignoré | 12 |

## Tests

**T2.1 — Ctrl-C sur prompt vide.**
```
minishell$ [Ctrl-C]
minishell$            # nouvelle ligne, prompt propre
```

**T2.2 — Ctrl-C avec texte tapé.**
```
minishell$ bonjour[Ctrl-C]
minishell$            # « bonjour » a disparu
```

**T2.3 — Ctrl-\ ne fait rien.**
```
minishell$ [Ctrl-\]
minishell$
```

**T2.4 — `$?` vaut 130 après Ctrl-C** (vérifiable dès la Phase 7 avec `echo $?`).

**T2.5 — Stress : Ctrl-C ×5 d'affilée** — cinq prompts propres, aucun crash.

---

# Phase 3 — Environnement (liste t_var depuis envp)

## Objectif

Lire `envp` dans notre liste chaînée `t_var`. `get_env_value`, `env_set`, `env_unset` fonctionnent.

## Concept : les trois états d'une variable

Une chaîne `"NAME=value"` contient un `=`. Une chaîne `"NAME"` sans `=` est « **déclarée sans valeur** » → `value = NULL`. La distinction est essentielle :

| Dans la liste | `env` l'affiche ? | `export` l'affiche ? | Passée à execve ? |
|---------------|:---:|:---:|:---:|
| `value="bar"` | ✅ `FOO=bar` | ✅ `declare -x FOO="bar"` | ✅ |
| `value=""` | ✅ `FOO=` | ✅ `declare -x FOO=""` | ✅ |
| `value=NULL` | ❌ | ✅ `declare -x FOO` | ❌ |

> 🔑 `value=NULL` ≠ `value=""`. Vérifie dans bash : `export FOO=` puis `export BAR` → `env | grep BAR` n'affiche rien, mais `export | grep BAR` affiche `declare -x BAR`.

## Qu'est-ce que envp ?

Le 3e paramètre de `main` : un tableau de chaînes `"NAME=value"` terminé par `NULL`, hérité du processus parent (bash) via `execve`. C'est un **instantané** de l'environnement au moment du lancement.

Pourquoi le copier dans une liste chaînée ? Parce qu'un tableau plat est pénible à **modifier** :

| Opération | `char **` | liste chaînée |
|-----------|-----------|---------------|
| ajouter (`export`) | realloc de tout le tableau | malloc d'un nœud |
| supprimer (`unset`) | décaler toute la queue | reconnecter 2 pointeurs |
| modifier (`cd` → PWD) | realloc si plus long | free + remplacer un champ |

## La structure

```c
typedef struct s_var
{
	char			*name;
	char			*value;   /* NULL = déclarée sans valeur */
	struct s_var	*next;
	struct s_var	*prev;    /* pour suppression O(1) dans unset */
}	t_var;
```

- `next`/`prev` sont des **pointeurs** (8 octets chacun), pas des structures imbriquées : une structure ne peut pas se contenir elle-même par valeur — la taille serait infinie (récursion). On écrit `struct s_var *` (et non `t_var *`) car le nom `t_var` n'existe qu'après l'accolade fermante du typedef.
- `prev` rend la suppression **O(1)** : connaissant le nœud, on reconnecte ses voisins sans re-parcourir la liste pour trouver « qui pointe vers moi ».

## Fichier env_init.c — construction de la liste

### new_var — une chaîne envp → un nœud

```c
t_var	*new_var(char *envp_line)
{
	t_var	*node;
	char	*eq;

	node = ft_calloc(1, sizeof(t_var));
	if (!node)
		return (NULL);
	eq = ft_strchr(envp_line, '=');
	if (eq)
	{
		node->name = ft_substr(envp_line, 0, eq - envp_line);
		node->value = ft_strdup(eq + 1);
		if (!node->name || !node->value)
			return (free(node->name), free(node->value), free(node), NULL);
	}
	else
	{
		node->name = ft_strdup(envp_line);
		if (!node->name)
			return (free(node), NULL);
	}
	return (node);
}
```

**`ft_calloc` et pas malloc :** calloc met la mémoire à zéro → `next`, `prev`, `value` valent déjà `NULL`. Avec malloc, ces champs contiendraient des déchets → segfault au premier parcours.

**`ft_strchr` — le PREMIER `=` :** la chaîne `"A=B=C"` doit donner `name="A"`, `value="B=C"`. Avec `ft_strrchr` (dernier `=`), on obtiendrait `name="A=B"` — faux.

**`eq - envp_line`** — arithmétique de pointeurs : le nombre de caractères entre le début et le `=`. Pour `"USER=yana"`, `eq` pointe sur `=` (index 4), la différence vaut 4 → `ft_substr(ligne, 0, 4)` = `"USER"`. Et `eq + 1` pointe juste après le `=` → `ft_strdup` copie `"yana"`.

**La chaîne de free avec l'opérateur virgule :** `return (free(a), free(b), free(c), NULL);` exécute de gauche à droite et retourne la dernière valeur. `free(NULL)` est sans danger (norme C) — pas besoin de if séparés. Idiome Norm-compatible qu'on retrouvera partout.

**Branche sans `=` :** `"NOVAL"` → tout dans `name`, `value` reste `NULL` (grâce à calloc).

### append_var — ajout en queue

```c
static void	append_var(t_var **head, t_var **tail, t_var *node)
{
	if (!*head)
	{
		*head = node;
		*tail = node;
		return ;
	}
	node->prev = *tail;
	(*tail)->next = node;
	*tail = node;
}
```

**Pourquoi des doubles pointeurs `t_var **` ?** Parce qu'on modifie les pointeurs eux-mêmes (le premier nœud devient la tête). Avec un simple `t_var *`, on modifierait une copie locale.

**Pourquoi un `tail` ?** Ajout en O(1). Sans tail, chaque ajout parcourt la liste → O(n²) pour construire (72² ≈ 5000 opérations au lieu de 72).

**`static`** — helper interne au fichier, personne d'autre n'en a besoin.

### create_env — la boucle principale

```c
t_var	*create_env(char **envp)
{
	t_var	*head;
	t_var	*tail;
	t_var	*node;
	int		i;

	head = NULL;
	tail = NULL;
	i = 0;
	while (envp && envp[i])
	{
		node = new_var(envp[i]);
		if (!node)
			return (ft_free_env(head), NULL);
		append_var(&head, &tail, node);
		i++;
	}
	return (head);
}
```

**`envp && envp[i]`** — double protection : envp lui-même peut être NULL (`env -i` extrême), et `envp[i] == NULL` marque la fin du tableau (convention POSIX).

**En cas d'échec de new_var :** `ft_free_env(head)` libère la partie déjà construite avant de retourner NULL. Pattern général du projet : *« si ça casse au milieu, nettoie ce que tu as construit »*.

## Fichier env_get.c — lecture

### get_env_value

```c
char	*get_env_value(char *name, t_var *env)
{
	while (env)
	{
		if (ft_strcmp(env->name, name) == 0)
			return (env->value);
		env = env->next;
	}
	return (NULL);
}
```

Recherche linéaire O(n). Retourne un pointeur **vers l'intérieur de la liste** — pas une copie. L'appelant ne doit **jamais** le libérer. Retourne NULL à la fois pour « variable absente » et « variable à value=NULL » — pour l'expansion (`$FOO`), les deux donnent une chaîne vide, donc c'est voulu. Pour les distinguer (dans `export`), on utilise `find_var`.

### convert_env_list — liste → char** pour execve

```c
static int	count_valued(t_var *env)      /* compte les value != NULL */
static char	*env_join(t_var *v)          /* "NAME" + "=" + "value"    */

char	**convert_env_list(t_var *env)
{
	char	**arr;
	int		i;

	arr = ft_calloc(count_valued(env) + 1, sizeof(char *));
	if (!arr)
		return (NULL);
	i = 0;
	while (env)
	{
		if (env->value)
		{
			arr[i] = env_join(env);
			if (!arr[i])
				return (ft_free_tab(arr), NULL);
			i++;
		}
		env = env->next;
	}
	return (arr);
}
```

**Pourquoi cette fonction existe :** `execve(path, argv, envp)` exige un `char **` au format `"NAME=value"`, terminé par NULL. Notre liste doit donc être re-convertie avant chaque lancement de commande externe (Phase 9).

**Les NULL-value sont sautées** — impossible de les représenter dans ce format, et bash fait pareil : une variable déclarée sans valeur n'est pas transmise aux enfants.

**`env_join` en deux ft_strjoin** — la libft n'a qu'un strjoin binaire ; on colle `"NAME"`+`"="` puis le résultat+`"value"`, en libérant l'intermédiaire.

**`+ 1` dans le calloc** — pour le NULL terminal qu'execve attend. Avec calloc, il est déjà là.

## Fichier env_set.c — modification

### env_set — LA fonction clé du module

```c
int	env_set(t_var **env, const char *name, char *value)
{
	t_var	*found;

	found = find_var(*env, name);
	if (found)
	{
		free(found->value);
		found->value = value;
		return (0);
	}
	return (env_append(env, name, value));
}
```

**Deux comportements en un :** si la variable existe → mise à jour (libérer l'ancienne valeur, brancher la nouvelle) ; sinon → création via `env_append`. C'est indispensable pour `cd` : `PWD` existe toujours (update), `OLDPWD` peut ne pas exister (create). Une fonction séparée « update only » qui échoue silencieusement quand la variable n'existe pas serait un bug sournois + une fuite.

> ⚠️ **`env_set` prend possession de `value`.** Après l'appel, ce pointeur ne t'appartient plus : soit il est attaché au nœud (libéré plus tard par `ft_free_env`), soit il est libéré immédiatement en cas d'échec malloc (`env_append` fait `free(value)` sur ses chemins d'erreur). Ne jamais faire `free(value)` après un `env_set`.

### env_remove / env_unset — suppression O(1)

```c
static void	env_remove(t_var **env, t_var *node)
{
	if (node->prev)
		node->prev->next = node->next;
	else
		*env = node->next;          /* on supprime la tête → nouvelle tête */
	if (node->next)
		node->next->prev = node->prev;
	free(node->name);
	free(node->value);
	free(node);
}
```

Quatre cas couverts par deux if :

| Cas | prev | next | Effet |
|-----|------|------|-------|
| milieu | ✅ | ✅ | les voisins se reconnectent entre eux |
| tête | ❌ | ✅ | `*env` avance, le nouveau premier a `prev=NULL` |
| queue | ✅ | ❌ | l'avant-dernier perd son next |
| unique | ❌ | ❌ | `*env = NULL`, liste vide |

`env_unset` cherche par nom (`find_var`) et supprime si trouvé. Un nom inexistant est **silencieusement ignoré** — comme bash (`unset NOPE ; echo $?` → 0).

## Tests

**T3.1 — La liste est complète.**
```
$ env | wc -l
50
$ ./minishell          # print_env (debug) affiche 50 lignes
```

**T3.2 — Environnement vide.**
```
$ env -i ./minishell   # 0 lignes, pas de crash
```

**T3.3 — Valgrind propre.**
```
$ valgrind --leak-check=full ./minishell
[Ctrl-D]  →  0 definitely lost
```

**T3.4 — PATH est correct.** Comparer avec `echo $PATH` dans bash.

**T3.5 à T3.8 — Tests unitaires** (petit main de test) :
- `env_set` crée puis met à jour : `bar` → `baz`, l'ancienne valeur est libérée
- `env_unset` sur la tête / le milieu / la queue / l'unique nœud
- `env_unset("INEXISTANT")` ne crashe pas
- `new_var("A=B=C")` → name=`A`, value=`B=C`

> 💡 `print_env` vit temporairement dans `utils/debug.c` — un fichier dédié aux fonctions de débogage, facile à retirer avant la soutenance.

---

# Phase 4 — Lexer

## Objectif

`tokenize_line(line, &err)` retourne une liste chaînée de `t_token` avec les types assignés et **les guillemets conservés**.

## Concept

> 🔑 Le lexer NE développe PAS les `$` et NE retire PAS les guillemets — c'est le rôle de `ft_expand` (Phase 7), en un seul passage. Le lexer ne fait que **découper**.

La subtilité principale vit dans `get_token_len` : **à l'intérieur des guillemets, les espaces et les opérateurs ne terminent pas le mot** :

```
echo "hi | there" > out
     └─────┬────┘
   UN SEUL token WORD : '"hi | there"'
   (le | entre guillemets est un simple caractère)
```

## Les deux structures

```c
typedef struct s_token              /* un token */
{
	char			*value;
	t_token_type	type;
	struct s_token	*next;
}	t_token;

typedef struct s_tok_list           /* wrapper : ajout en queue O(1) */
{
	t_token	*head;
	t_token	*tail;
}	t_tok_list;
```

**Pourquoi deux structures ?** Séparation des responsabilités : `s_token` porte les données d'**un** token ; `s_tok_list` gère la liste **entière** (accès direct à la tête et à la queue). Le wrapper ne vit que pendant `tokenize_line` — seule la tête sort de la fonction.

Les types de tokens :

```c
typedef enum e_token_type
{
	WORD = 0,
	REDIR_IN,       /* <  */
	REDIR_OUT,      /* >  */
	REDIR_APPEND,   /* >> */
	HEREDOC,        /* << */
	PIPE            /* |  */
}	t_token_type;
```

## Le code — lexer.c

```c
#include "minishell.h"

int	is_special(char c)
{
	return (c == '|' || c == '<' || c == '>');
}

int	is_whitespace(char c)
{
	return (c == ' ' || c == '\t');
}

/* Longueur du segment entre guillemets, guillemet fermant inclus.
   -1 si non fermé. */
static int	quoted_len(char *s)
{
	char	q;
	int		i;

	q = s[0];
	i = 1;
	while (s[i] && s[i] != q)
		i++;
	if (!s[i])
		return (-1);
	return (i + 1);
}

int	get_token_len(char *s)
{
	int	i;
	int	q;

	if (is_special(s[0]))
	{
		if ((s[0] == '<' && s[1] == '<') || (s[0] == '>' && s[1] == '>'))
			return (2);
		return (1);
	}
	i = 0;
	while (s[i] && !is_whitespace(s[i]) && !is_special(s[i]))
	{
		if (s[i] == '\'' || s[i] == '"')
		{
			q = quoted_len(s + i);
			if (q < 0)
				return (-1);
			i += q;
		}
		else
			i++;
	}
	return (i);
}

static int	process_one_token(char **p, t_tok_list *list)
{
	int		len;
	char	*sub;

	len = get_token_len(*p);
	if (len <= 0)
		return (0);
	sub = extract_quoted(*p, len);
	if (!sub)
		return (0);
	if (!add_token(sub, assign_type(*p), list))
		return (free(sub), 0);
	*p += len;
	return (1);
}

t_token	*tokenize_line(char *line, int *err)
{
	t_tok_list	list;
	char		*p;

	*err = 0;
	list.head = NULL;
	list.tail = NULL;
	p = line;
	while (*p)
	{
		while (*p && is_whitespace(*p))
			p++;
		if (!*p)
			break ;
		if (!process_one_token(&p, &list))
		{
			free_token_list(list.head);
			*err = 2;
			ft_putendl_fd("minishell: syntax error: unclosed quote",
				STDERR_FILENO);
			return (NULL);
		}
	}
	return (list.head);
}
```

## Explication fonction par fonction

### quoted_len

`q = s[0]` mémorise **quel** guillemet ouvre (`'` ou `"`) — on ne cherche que le même en fermeture (`"a'b"` est valide : le `'` intérieur est un simple caractère). Le compteur part de `i = 1` (on saute l'ouvrant). Si la boucle atteint `'\0'` sans trouver le fermant → **-1** (guillemet non fermé). Sinon `i + 1` : la longueur totale, ouvrant et fermant inclus.

Exemples : `"abc"` → 5 · `''` → 2 · `"x` → -1 · `"$U"trail` → 4 (on s'arrête au fermant, la suite sera traitée par l'appel suivant dans la boucle).

### get_token_len

**Branche opérateurs :** si `s[0]` est spécial → longueur 1, ou 2 pour les doubles `<<` / `>>`.

**Branche mot :** on avance tant que le caractère n'est ni fin de chaîne, ni espace, ni opérateur. **Sauf** si on rencontre un guillemet : là, on saute d'un bloc entier via `quoted_len` — c'est ce saut qui fait que les espaces et opérateurs *à l'intérieur* des guillemets ne coupent pas le mot. Le -1 de quoted_len est propagé vers le haut.

Exemples : `echo hi` → 4 · `"hi there"` → 10 · `"a"b"c"` → 7 (trois segments collés) · `hello|wc` → 5 · `"unclosed` → -1.

### assign_type — le type d'après le PREMIER caractère

| s[0] | s[1] | Type |
|------|------|------|
| `\|` | — | PIPE |
| `<` | `<` | HEREDOC |
| `>` | `>` | REDIR_APPEND |
| `<` | autre | REDIR_IN |
| `>` | autre | REDIR_OUT |
| autre (y compris `"`, `'`, `$`) | — | WORD |

Conséquence élégante : `echo "|"` → le token commence par `"` → **WORD**. Le pipe entre guillemets n'est jamais un opérateur, sans aucun code supplémentaire.

### process_one_token — l'assembleur

Quatre étapes : longueur (`get_token_len`) → copie (`extract_quoted`) → type (`assign_type`) → ajout (`add_token`), puis avancer le pointeur `*p += len`. Chaque étape peut échouer → retour 0, et le nettoyage remonte à l'appelant. Le double pointeur `char **p` permet de faire avancer le **vrai** curseur de l'appelant, pas une copie.

### tokenize_line — la boucle maîtresse

Le patron classique du lexing : *sauter les séparateurs → traiter un token → recommencer*. Deux sorties NULL bien distinctes :

| Situation | Retour | `*err` | Message |
|-----------|--------|--------|---------|
| guillemet non fermé / échec malloc | NULL | 2 | `unclosed quote` sur stderr |
| ligne vide ou espaces seulement | NULL | 0 | rien (pas une erreur !) |

> ⚠️ C'est pour distinguer ces deux cas que le paramètre out `int *err` existe. Dans `run_line` : `if (!tokens) return (err ? err : status);`

En cas d'erreur au milieu, `free_token_list(list.head)` libère les tokens déjà construits — sinon fuite.

## Tests

**T4.1 — Mots simples.**
```
minishell$ echo hello world
[WORD: 'echo'] [WORD: 'hello'] [WORD: 'world']
```

**T4.2 — Les guillemets collent (et sont conservés !).**
```
minishell$ echo "hi there"
[WORD: 'echo'] [WORD: '"hi there"']
```

**T4.3 — Opérateurs, avec ou sans espaces.**
```
minishell$ ls -l|wc
[WORD: 'ls'] [WORD: '-l'] [PIPE: '|'] [WORD: 'wc']
minishell$ cat < in >> out
[WORD: 'cat'] [REDIR_IN: '<'] [WORD: 'in'] [REDIR_APPEND: '>>'] [WORD: 'out']
```

**T4.4 — Heredoc collé.**
```
minishell$ cat <<EOF
[WORD: 'cat'] [HEREDOC: '<<'] [WORD: 'EOF']
```

**T4.5 — Opérateur entre guillemets = WORD.**
```
minishell$ echo "|"
[WORD: 'echo'] [WORD: '"|"']
```

**T4.6 — Guillemet non fermé.**
```
minishell$ echo 'hello
minishell: syntax error: unclosed quote      # $? interne = 2
```

**T4.7 — Segments collés.**
```
minishell$ echo "a"b"c"
[WORD: 'echo'] [WORD: '"a"b"c"']             # UN seul token
```

**T4.8 — Le `$` n'est PAS développé.**
```
minishell$ echo $USER
[WORD: 'echo'] [WORD: '$USER']
```

**T4.9 — Ligne d'espaces : silencieux, `$?` inchangé.**

**T4.10 — Valgrind sur T4.6 : 0 fuites** (la liste partielle est libérée).

---

# Phase 5 — Syntaxe

## Objectif

`syntax_ok` retourne 0 sur une séquence cassée, affiche `` minishell: syntax error near unexpected token `X' `` et met `$? = 2`.

## Pourquoi une vérification séparée ?

Le lexer produit des tokens, mais ils peuvent former des séquences **absurdes** dont aucun arbre ne peut naître :

```
| ls          ← pipe sans partie gauche
ls |          ← pipe sans partie droite
ls || wc      ← deux pipes de suite (|| est un bonus, pas dans le mandatory)
cat <         ← redirection sans nom de fichier
cat > |       ← redirection suivie d'un pipe au lieu d'un fichier
```

Laisser passer ça vers le parseur = arbre difforme ou crash. Plus simple de rejeter **avant**, exactement comme bash :

```
$ | ls
bash: syntax error near unexpected token `|'
$ echo $?
2
```

## Prototypes

```c
int	syntax_ok(t_token *t, int *error_code);
int	syntax_err(const char *tok, int *error_code);
```

- `syntax_ok` : 1 = syntaxe correcte, 0 = erreur (avec `*error_code = 2`)
- `syntax_err` : affiche le message et retourne **toujours 0** → l'idiome `return (syntax_err(...));` fait l'affichage et le « faux » en une ligne

## Le code — syntax.c

```c
#include "minishell.h"

static int	is_redir(t_token_type t)
{
	return (t == REDIR_IN || t == REDIR_OUT
		|| t == REDIR_APPEND || t == HEREDOC);
}

int	syntax_err(const char *tok, int *error_code)
{
	ft_putstr_fd("minishell: syntax error near unexpected token `", 2);
	ft_putstr_fd((char *)tok, 2);
	ft_putendl_fd("'", 2);
	if (error_code)
		*error_code = 2;
	return (0);
}

int	syntax_ok(t_token *t, int *error_code)
{
	if (error_code)
		*error_code = 0;
	if (t && t->type == PIPE)
		return (syntax_err("|", error_code));
	while (t)
	{
		if (t->type == PIPE && (!t->next || t->next->type == PIPE))
			return (syntax_err("|", error_code));
		if (is_redir(t->type) && (!t->next || t->next->type != WORD))
			return (syntax_err(
					t->next ? t->next->value : "newline", error_code));
		t = t->next;
	}
	return (1);
}
```

## syntax_err — les détails qui comptent

1. **fd 2 = stderr.** Les erreurs vont toujours sur stderr : avec `./minishell > log`, elles restent visibles à l'écran au lieu de partir dans le fichier.
2. **Trois appels d'affichage** — début du message, le token, l'apostrophe fermante avec saut de ligne. On compose par morceaux car il n'y a pas de printf-vers-stderr dans la libft.
3. **`(char *)tok`** — cast retirant le const, car `ft_putstr_fd` est déclarée `(char *, int)`. Inoffensif : la fonction ne modifie pas la chaîne.
4. **`if (error_code) *error_code = 2;`** — même pattern d'out-paramètre que dans `tokenize_line`, avec protection NULL.
5. **`return (0)`** — toujours faux, pour l'idiome une-ligne.

## syntax_ok — les trois vérifications

**Vérification n°1 — PIPE en tout premier token** (avant la boucle) :

```c
if (t && t->type == PIPE)
	return (syntax_err("|", error_code));
```

`| ls` : le pipe n'a pas de commande à gauche. On le teste à part car dans la boucle on regarde les paires « courant + suivant », et « pas de précédent » se teste plus naturellement sur le premier. Le `t &&` protège du cas liste NULL.

**Vérification n°2 — PIPE en dernier, ou deux PIPE consécutifs :**

```c
if (t->type == PIPE && (!t->next || t->next->type == PIPE))
	return (syntax_err("|", error_code));
```

- `ls |` → PIPE dernier, `t->next == NULL`. (Le vrai bash attend une suite avec le prompt secondaire `>` ; en minishell, on rejette — comportement accepté en soutenance.)
- `ls || wc` → le lexer a découpé `||` en deux tokens `|` ; on les attrape ici. (`||` est un opérateur du bonus.)

**Vérification n°3 — redirection sans cible :**

```c
if (is_redir(t->type) && (!t->next || t->next->type != WORD))
	return (syntax_err(t->next ? t->next->value : "newline", error_code));
```

Après `<`, `>`, `>>`, `<<`, un **WORD est obligatoire** (nom de fichier ou délimiteur heredoc). Le ternaire reproduit le format bash :

- `cat <` → pas de suivant → `` ...token `newline' `` (c'est le mot que bash utilise pour la fin de ligne)
- `cat < |` → le suivant existe et c'est `|` → `` ...token `|' ``

## Pourquoi ces trois vérifications suffisent

Après elles, **toute** séquence de nos six types de tokens est garantie analysable :

- des WORD consécutifs = une commande avec arguments : `ls -l -a` ✓
- chaque redirection a sa cible WORD ✓
- chaque PIPE a quelque chose des deux côtés ✓

Aucune autre combinaison invalide n'existe. La vérification est complète.

## Intégration dans run_line

```c
static int	run_line(char *line, t_var *env, int status)
{
	t_token	*tokens;
	int		err;

	(void)env;
	tokens = tokenize_line(line, &err);
	if (!tokens)
		return (err ? err : status);
	if (!syntax_ok(tokens, &err))
		return (free_token_list(tokens), err);
	print_tokens(tokens);           /* debug — remplacé en Phase 6 */
	free_token_list(tokens);
	return (status);
}
```

> ⚠️ `free_token_list(tokens)` **avant** le retour d'erreur : les tokens sont déjà construits, il faut les libérer. Toujours le même pattern : *ça casse → on nettoie derrière soi*.

## Tests

**T5.1 — Pipe au début / à la fin / doublé.**
```
minishell$ | ls
minishell: syntax error near unexpected token `|'
minishell$ ls |
minishell: syntax error near unexpected token `|'
minishell$ ls || wc
minishell: syntax error near unexpected token `|'
```

**T5.2 — Redirection sans cible.**
```
minishell$ ls >
minishell: syntax error near unexpected token `newline'
minishell$ cat < |
minishell: syntax error near unexpected token `|'
minishell$ > > out
minishell: syntax error near unexpected token `>'
```

**T5.3 — Le valide passe.**
```
minishell$ ls -l | grep foo > out
[WORD: 'ls'] [WORD: '-l'] [PIPE: '|'] [WORD: 'grep'] ...
```

**T5.4 — Chaque message = format bash** (comparer côte à côte, seul le préfixe change).

**T5.5 — Valgrind sur tous les cas d'erreur : 0 fuites.**

> 💡 Après cette phase, `echo $?` n'affiche pas encore 2 — le `$?` ne sera développé qu'en Phase 7. Mais le `status` interne est déjà correct (vérifiable avec un printf temporaire dans la boucle main).

---

# Carte transversale des données (Phases 0–5)

```
ligne de readline
      │  tokenize_line          (Phase 4)
      ▼
liste t_token  ──✗── NULL + err=2 (unclosed quote)
      │  syntax_ok              (Phase 5)
      ▼
   1 = ok ──✗── 0 + err=2 (message sur stderr)
      │
      ▼
  [Phase 6 : parsing → arbre t_node]   ← PROCHAINE ÉTAPE
```

## Propriété mémoire — récapitulatif

| Objet | Créé par | Libéré par | Quand |
|-------|----------|------------|-------|
| `line` | readline | main | fin de chaque itération |
| liste `t_var` | create_env | ft_free_env | sortie du shell |
| liste `t_token` | tokenize_line | run_line → free_token_list | après usage ou sur erreur |
| historique | add_history | rl_clear_history | sortie du shell |

## Codes de retour — cumulés

| Code | Signification | Introduit en |
|------|---------------|--------------|
| 0 | succès | Phase 0 |
| 2 | erreur de syntaxe | Phases 4–5 |
| 130 | interrompu par Ctrl-C (128 + 2) | Phase 2 |
