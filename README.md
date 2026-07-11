
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
# 🐚 minishell — documentation (Phases 0–5)

> Projet : minishell (École 42) · Architecture : AST · Auteur : yyuskiv
Statut : ✅ Phases 0–5 terminées · 🔜 Phase 6 — parseur
> 

---

# Phase 0 — Setup & Makefile

## Objectif

Le projet compile. `make`, `clean`, `fclean`, `re` fonctionnent. Un `main` vide retourne 0.

## Décisions clés

- **Makefile avec dépendances** — les flags `MMD -MP` génèrent des fichiers `.d` : toute modification de `minishell.h` recompile tout ce qui l'inclut.
- **macOS + Apple Silicon :** le readline système est libedit, qui ne contient pas `rl_replace_line`. On utilise GNU Readline via brew :

```makefile
READLINE_PREFIX := $(shell brew --prefix readline)
INCLUDES := -I. -Ilibft -I$(READLINE_PREFIX)/include
LDLIBS   := -L$(READLINE_PREFIX)/lib -lreadline -Llibft -lft
```

> 💡 Sur les machines Linux de l'école, `-lreadline` suffit. Un Makefile universel se fait avec `ifeq ($(shell uname -s),Darwin)`.
> 

## Tests

| # | Commande | Attendu |
| --- | --- | --- |
| T0.1 | `make` | compilation sans warnings |
| T0.2 | `./minishell && echo $?` | `0` |
| T0.3 | `make clean && make fclean && make re` | tout se recompile |
| T0.4 | `make && make` | deuxième fois : « Nothing to be done » (pas de relink) |
| T0.5 | `touch minishell.h && make` | tous les .o se recompilent (les fichiers .d fonctionnent) |

---

# Phase 1 — Boucle REPL

## Objectif

Prompt `minishell$`, readline lit une ligne, Ctrl-D quitte avec `exit`.

## Concept

**REPL** = Read–Eval–Print–Loop. Chaque itération est *un univers séparé* : les objets malloc'és dans une itération doivent mourir à la fin de celle-ci.

> 🧠 Réflexe principal : **je reçois un pointeur → je sais immédiatement qui va le libérer.**
> 

## Structure de la boucle

```c
while (1)
{
    setup_signal_handlers();          /* à chaque itération */
    line = readline("minishell$ ");   /* malloc à l'intérieur ! */
    if (!line)                        /* NULL = Ctrl-D (EOF) */
        break ;
    status = post_readline_status(status);
    if (*line)                        /* lignes vides pas dans l'historique */
        add_history(line);
    status = run_line(line, env, status);
    free(line);                       /* mémoire de readline */
}
```

## Propriété de la mémoire

| Pointeur | Qui alloue | Qui libère |
| --- | --- | --- |
| `line` | readline (malloc) | `main` → `free(line)` à chaque itération |
| historique | readline (interne) | `rl_clear_history()` avant de quitter |

## Tests

| # | Action | Attendu |
| --- | --- | --- |
| T1.1 | `./minishell` → Ctrl-D | affiche `exit`, `$?` = 0 |
| T1.2 | Enter sur ligne vide ×3 | nouveaux prompts, pas de crash |
| T1.3 | `hello` + Enter | le stub affiche la ligne |
| T1.4 | ligne d'espaces + Enter | prompt, pas dans l'historique |
| T1.5 | ↑ (flèche haut) | affiche la commande précédente |
| T1.6 | `valgrind ./minishell` → Ctrl-D | 0 definitely lost |

---

# Phase 2 — Signaux (contexte prompt)

## Objectif

Ctrl-C au prompt — nouvelle ligne, prompt propre, `$? = 130`. Ctrl-\ est ignoré.

## Concept : `volatile sig_atomic_t`

Les signaux sont des **interruptions asynchrones** : le processeur peut arrêter le code principal à n'importe quelle nanoseconde pour exécuter le handler.

`extern volatile sig_atomic_t g_sig;` — le seul canal sûr « handler → code principal » :

| Mot-clé | Ce qu'il apporte | Sans lui |
| --- | --- | --- |
| `sig_atomic_t` | écriture en une seule instruction CPU (atomicité) | le handler peut lire une « bouillie » de moitié ancienne / moitié nouvelle valeur |
| `volatile` | lecture toujours depuis la RAM, pas depuis un registre | `while (g_sig == 0)` devient une boucle infinie après optimisation |
| `extern` | une seule variable pour tous les fichiers .c | multiple definition à l'édition de liens |

> ⚠️ Sujet 42 : **une seule** globale, contenant **uniquement le numéro du signal**. Ni env, ni status dans les globales.
> 

## setup_signal_handlers — ligne par ligne

```c
void	setup_signal_handlers(void)
{
	struct sigaction	sa;

	sa.sa_handler = on_signal_prompt;  /* notre fonction */
	sigemptyset(&sa.sa_mask);          /* on ne bloque rien */
	sa.sa_flags = 0;                   /* SANS SA_RESTART ! */
	sigaction(SIGINT, &sa, NULL);
	signal(SIGQUIT, SIG_IGN);          /* Ctrl-\ ignoré */
}
```

> 🔑 **Pourquoi `sa_flags = 0` :** le flag `SA_RESTART` relancerait les appels système interrompus. On veut l'inverse — que `readline` soit **interrompue** par Ctrl-C et rende la main.
> 

## Le handler

```c
static void	on_signal_prompt(int signo)
{
	g_sig = signo;
	if (signo == SIGINT)
	{
		write(STDOUT_FILENO, "\n", 1);
		rl_on_new_line();        /* readline : nouvelle ligne */
		rl_replace_line("", 0);  /* vider le buffer de saisie */
		rl_redisplay();          /* redessiner le prompt */
	}
}
```

## Trois contextes de signaux (carte pour la suite)

| Contexte | SIGINT | SIGQUIT | Quand |
| --- | --- | --- | --- |
| prompt | new line + redraw | ignore | Phase 2 ✅ |
| attente d'un enfant | ignore (l'enfant le reçoit lui-même) | ignore | Phase 9 |
| heredoc | close(stdin) → interrompre readline | ignore | Phase 12 |

## Tests

| # | Action | Attendu |
| --- | --- | --- |
| T2.1 | Ctrl-C sur prompt vide | nouvelle ligne, prompt propre |
| T2.2 | `bonjourCtrl-C` | le texte disparaît, prompt propre |
| T2.3 | Ctrl-\ | rien ne se passe |
| T2.4 | Ctrl-C, puis `echo $?` (après Phase 7) | `130` |
| T2.5 | Ctrl-C ×5 d'affilée | cinq prompts propres, pas de crash |

---

# Phase 3 — Environnement (t_var depuis envp)

## Objectif

envp copié dans une liste chaînée `t_var`. `get_env_value`, `env_set`, `env_unset` fonctionnent.

## Concept : trois états d'une variable

| Dans la liste | `env` l'affiche ? | `export` l'affiche ? | Dans envp pour execve ? |
| --- | --- | --- | --- |
| `name="FOO"`, `value="bar"` | ✅ `FOO=bar` | ✅ `declare -x FOO="bar"` | ✅ |
| `name="FOO"`, `value=""` | ✅ `FOO=` | ✅ `declare -x FOO=""` | ✅ |
| `name="FOO"`, `value=NULL` | ❌ | ✅ `declare -x FOO` | ❌ |

> 🔑 `value=NULL` ≠ `value=""`. Le premier — « déclarée sans valeur » (`export FOO`), le second — « valeur vide » (`export FOO=`).
> 

## Structure

```c
typedef struct s_var
{
	char			*name;
	char			*value;   /* NULL = déclarée sans valeur */
	struct s_var	*next;
	struct s_var	*prev;    /* pour suppression O(1) dans unset */
}	t_var;
```

- `next`/`prev` sont des **pointeurs** (8 octets), pas des structures : une structure ne peut pas se contenir elle-même par valeur (récursion infinie des tailles).
- `prev` sert à `env_remove` : extraire un nœud du milieu en O(1) sans re-chercher « qui pointe vers moi ».

## API du module

| Fonction | Rôle | Propriété mémoire |
| --- | --- | --- |
| `create_env(envp)` | envp → liste | copie les chaînes (substr/strdup) |
| `get_env_value(name, env)` | valeur par nom | retourne un pointeur **dans** la liste — NE PAS libérer |
| `env_set(&env, name, value)` | mettre à jour OU créer | **prend possession** de value |
| `env_unset(&env, name)` | supprimer par nom | ignore silencieusement l'inexistant (comme bash) |
| `convert_env_list(env)` | liste → char** pour execve | saute les NULL-value |
| `ft_free_env(env)` | tout libérer | name + value + nœud, pour chacun |

> ⚠️ **`env_set` prend possession de value.** Après `env_set(&env, "PWD", new_pwd)`, le pointeur `new_pwd` n'est plus à toi — ni free, ni utilisation.
> 

## Découpage de la chaîne dans new_var

```
"USER=yana"          "A=B=C"              "NOVAL"
name="USER"          name="A"             name="NOVAL"
value="yana"         value="B=C"          value=NULL
     ↑                    ↑
ft_strchr — PREMIER '='  (pas strrchr !)
```

## Tests

| # | Commande | Attendu |
| --- | --- | --- |
| T3.1 | `env | wc -l` vs comptage print_env | nombres identiques |
| T3.2 | `env -i ./minishell` | 0 lignes, pas de crash |
| T3.3 | `valgrind ./minishell` → Ctrl-D | 0 definitely lost |
| T3.4 | trouver `PATH` dans print_env | identique à `echo $PATH` dans bash |
| T3.5 | unitaire : `env_set` crée → met à jour → la valeur change | `bar` → `baz` |
| T3.6 | unitaire : `env_unset` tête / milieu / queue / unique | liste correcte |
| T3.7 | unitaire : `env_unset("NOPE")` sur liste vide | pas de crash |
| T3.8 | unitaire : `new_var("A=B=C")` | name=`A`, value=`B=C` |

---

# Phase 4 — Lexer

## Objectif

`tokenize_line(line, &err)` → liste chaînée de `t_token` avec types. Les guillemets sont **conservés** dans la valeur.

## Concept

> 🔑 Le lexer NE développe PAS les `$` et NE retire PAS les guillemets — c'est `ft_expand` (Phase 7) qui le fera en un seul passage. Le rôle du lexer : uniquement **découper** la chaîne et assigner les types.
> 

La subtilité principale dans `get_token_len` : **à l'intérieur des guillemets, les espaces et opérateurs NE terminent PAS le mot** :

```
echo "hi | there" > out
     └────┬─────┘
     UN SEUL token WORD : '"hi | there"'
     (le | entre guillemets n'est qu'un caractère)
```

## Deux structures

```c
typedef struct s_token          /* un token */
{
	char			*value;
	t_token_type	type;
	struct s_token	*next;
}	t_token;

typedef struct s_tok_list       /* wrapper pour ajout en queue en O(1) */
{
	t_token	*head;
	t_token	*tail;
}	t_tok_list;
```

`t_tok_list` ne vit qu'à l'intérieur de `tokenize_line` — seul `head` sort.

## Pipeline de fonctions

```
tokenize_line                    boucle principale : skip espaces → process token
 └─ process_one_token            len → extract → type → add → advance
     ├─ get_token_len            longueur du token (les guillemets collent !)
     │   └─ quoted_len           longueur du segment "...", -1 si non fermé
     ├─ extract_quoted           copie malloc du segment
     ├─ assign_type              type selon le PREMIER caractère
     └─ add_token                nœud en queue de liste
```

## assign_type — type selon le premier caractère

| s[0] | s[1] | Type |
| --- | --- | --- |
| `|` | — | PIPE |
| `<` | `<` | HEREDOC |
| `>` | `>` | REDIR_APPEND |
| `<` | autre | REDIR_IN |
| `>` | autre | REDIR_OUT |
| autre (incl. `"` `'` `$`) | — | WORD |

## Gestion des erreurs

- Guillemet non fermé → `quoted_len` retourne -1 → `get_token_len` retourne -1 → tout remonte à `tokenize_line`, qui : affiche `minishell: syntax error: unclosed quote` (stderr), écrit `err = 2`, libère la liste partielle, retourne NULL.
- Ligne vide → NULL avec `err = 0` (pas une erreur !).

> ⚠️ Dans `run_line` : `tokens == NULL` recouvre DEUX cas différents. On les distingue par `err`.
> 

## Tests

| # | Saisie | Attendu |
| --- | --- | --- |
| T4.1 | `echo hello world` | `[WORD echo] [WORD hello] [WORD world]` |
| T4.2 | `echo "hi there"` | `[WORD echo] [WORD "hi there"]` — guillemets présents ! |
| T4.3 | `ls -l|wc` | `[WORD ls] [WORD -l] [PIPE] [WORD wc]` — sans espaces |
| T4.4 | `cat < in >> out` | `[WORD cat] [REDIR_IN] [WORD in] [REDIR_APPEND] [WORD out]` |
| T4.5 | `cat <<EOF` | `[WORD cat] [HEREDOC] [WORD EOF]` |
| T4.6 | `echo "|"` | `[WORD echo] [WORD "|"]` — pipe entre guillemets = WORD |
| T4.7 | `echo 'hello` | `unclosed quote`, `$?` interne = 2 |
| T4.8 | `echo "a"b"c"` | un seul token `"a"b"c"` — les guillemets collent |
| T4.9 |  (espaces) | liste vide, silencieux, `$?` inchangé |
| T4.10 | `echo $USER` | `[WORD echo] [WORD $USER]` — `$` PAS développé |

---

# Phase 5 — Syntaxe

## Objectif

`syntax_ok` rejette les séquences cassées avec un message au format bash et `$? = 2`.

## Pourquoi une vérification séparée

Les tokens peuvent former des séquences absurdes, dont aucun arbre ne peut être construit :

```
| ls          ← pipe sans partie gauche
ls |          ← pipe sans partie droite
ls || wc      ← deux pipes de suite (|| c'est le bonus, on ne l'a pas)
cat <         ← redirection sans nom de fichier
cat > |       ← redirection suivie d'un pipe au lieu d'un fichier
```

Plus simple de rejeter **avant** le parseur — comme bash : message + `$? = 2`.

## Trois vérifications — et pourquoi elles suffisent

```c
/* n°1 : PIPE en premier token */
if (t && t->type == PIPE)
	return (syntax_err("|", error_code));

/* n°2 : PIPE en dernier ou deux PIPE de suite */
if (t->type == PIPE && (!t->next || t->next->type == PIPE))
	return (syntax_err("|", error_code));

/* n°3 : redirection sans cible WORD */
if (is_redir(t->type) && (!t->next || t->next->type != WORD))
	return (syntax_err(t->next ? t->next->value : "newline", error_code));
```

Après ces trois vérifications, **toute** séquence des six types de tokens est garantie analysable par le parseur :

- des WORD consécutifs = commande avec arguments ✓
- chaque redirection a une cible WORD ✓
- chaque PIPE a ses deux côtés ✓

## syntax_err — détails

- Affichage sur **stderr** (fd 2) — avec `./minishell > log`, les erreurs restent à l'écran.
- Retourne **toujours 0** → idiome `return (syntax_err(...))` — affichage et « faux » en une ligne.
- Format identique à bash : `minishell: syntax error near unexpected token `X'`
- `cat <` → pas de token suivant → on affiche `newline` (c'est ainsi que bash désigne la fin de ligne).

## Tests

| # | Saisie | Message | `$?` |
| --- | --- | --- | --- |
| T5.1 | `| ls` | `...token `|'` | 2 |
| T5.2 | `ls |` | `...token `|'` | 2 |
| T5.3 | `ls || wc` | `...token `|'` | 2 |
| T5.4 | `ls >` | `...token `newline'` | 2 |
| T5.5 | `cat < |` | `...token `|'` | 2 |
| T5.6 | `> > out` | `...token `>'` | 2 |
| T5.7 | `<< | EOF` | `...token `|'` | 2 |
| T5.8 | `ls -l | grep foo > out` | (les tokens s'affichent) | inchangé |
| T5.9 | `echo | | echo` | `...token `|'` | 2 |
| T5.10 | valgrind sur T5.1–T5.7 | 0 fuites (free_token_list a fonctionné) | — |

> ✅ Compare chaque message avec le vrai bash — le format doit correspondre (sauf le préfixe `bash:` / `minishell:`).
> 

---

# Carte transversale des données (Phases 0–5)

```
ligne de readline
      │  tokenize_line          (Phase 4)
      ▼
liste t_token  ──✗── NULL + err=2 (unclosed quote)
      │  syntax_ok              (Phase 5)
      ▼
   1 = ok ──✗── 0 + err=2 (affichage sur stderr)
      │
      ▼
  [Phase 6 : parsing → arbre t_node]   ← PROCHAINE ÉTAPE
```

## Propriété mémoire — récapitulatif

| Objet | Créé par | Libéré par | Quand |
| --- | --- | --- | --- |
| `line` | readline | main | fin de chaque itération |
| liste `t_var` | create_env | ft_free_env | sortie du shell |
| liste `t_token` | tokenize_line | run_line → free_token_list | après le parsing ou en cas d'erreur |
| historique | add_history | rl_clear_history | sortie du shell |

## Codes de retour — cumulés

| Code | Signification | Introduit en |
| --- | --- | --- |
| 0 | succès | Phase 0 |
| 2 | erreur de syntaxe | Phases 4–5 |
| 130 | interrompu par Ctrl-C (128+2) | Phase 2 |
