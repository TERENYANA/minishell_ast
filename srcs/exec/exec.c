#include "../minishell.h"

/*
** FUNCTION: run_builtin_in_parent
** --------------------------------
** Exécute les commandes internes du shell (comme `cd`, `exit`, `export`,
	`unset`)
** directement dans le processus parent au lieu de créer un processus enfant.
**
** POURQUOI LE PROCESSUS PARENT ?
** Les built-ins modifient l'état du shell (ex: changement du dossier de travail ou de l'environnement).
** S'ils étaient exécutés dans un processus enfant,
	ces changements seraient perdus à la fin de l'enfant.
**
** Param: node       
	- Pointeur vers le nœud AST contenant la commande et ses arguments.
** Param: env        
	- Pointeur vers le pointeur de la structure d'environnement.
** Param: last_status - Statut de sortie de la commande précédente (pour `$?`).
**
** Return: Entier - le code de statut renvoyé par la commande built-in exécutée.
**
** EXEMPLE DE TRACE (`cd /tmp`):
** 1. Vérifie si `node->cmd` ou `node->cmd[0]` ("cd") est NULL (renvoie 0 si vide).
** 2. Appelle `dispatch_builtin(node, env, last_status)`.
** 3. `dispatch_builtin` exécute `chdir("/tmp")` et met à jour `env`.
** 4. Renvoie 0 (succès).
*/
static int	run_builtin_in_parent(t_node *node, t_var **env, int last_status)
{
	int	saved_in;
	int	saved_out;
	int	ret;

	saved_in = dup(STDIN_FILENO);
	saved_out = dup(STDOUT_FILENO);
	if (saved_in < 0 || saved_out < 0)
	{
		if (saved_in >= 0)
			close(saved_in);
		if (saved_out >= 0)
			close(saved_out);
		return (1);
	}
	if (apply_redirections(node) != 0)
		ret = 1;
	else if (!node->cmd || !node->cmd[0])
		ret = 0;
	else
		ret = dispatch_builtin(node, env, last_status);
	dup2(saved_in, STDIN_FILENO);
	dup2(saved_out, STDOUT_FILENO);
	close(saved_in);
	close(saved_out);
	return (ret);
}

/*
** FUNCTION: is_parent_builtin_root
** ---------------------------------
** Fonction de vérification (prédicat) qui détermine si le nœud racine actuel de l'AST
** représente une simple commande built-in qui DOIT s'exécuter dans le shell parent.
**
** Param: root - Nœud racine de l'AST à vérifier.
**
** Return: 1 (vrai) si la racine est une commande simple ET que son binaire est un built-in.
**         0 (faux) si c'est un programme externe (ex: `ls`,
	`grep`) ou un pipeline complexe.
**
** EXEMPLE DE TRACE :
** Entrée : `export VAR=42`
** - `root->type == N_CMD` (Nœud de commande simple)
** - `root->cmd[0] = "export"`
** - `is_builtin("export")` renvoie 1 -> La fonction renvoie 1 (vrai).
*/
static int	is_parent_builtin_root(t_node *root)
{
	return (root->type == N_CMD && root->cmd && root->cmd[0]
		&& is_builtin(root->cmd[0]));
}

/*
** FUNCTION: run_tree
** -------------------
** Gestionnaire principal de l'exécution de l'AST (Arbre Syntaxique Abstrait).
** Décide s'il faut exécuter dans le processus parent (pour les built-ins seuls)
** ou créer un processus enfant (fork) pour les programmes externes.
**
** Param: root        - Nœud racine de l'AST.
** Param: env        
	- Pointeur vers le pointeur de la structure d'environnement.
** Param: last_status - Code de sortie de la commande précédente.
**
** Return: Code de sortie de la commande exécutée (0-255).
**
** EXEMPLE DE TRACE 1 (Exécution de `cd /usr` - Chemin parent) :
** 1. `is_parent_builtin_root(root)` vérifie si "cd" est un built-in
	-> renvoie 1 (vrai).
** 2. Exécute directement `run_builtin_in_parent(...)` et renvoie son résultat.
**
** EXEMPLE DE TRACE 2 (Exécution de `ls -l` - Chemin fork) :
** 1. `is_parent_builtin_root(root)` renvoie 0.
** 2. `ignore_signals()` empêche le shell parent de réagir aux signaux pendant l'exécution.
** 3. `pid = fork()` crée le processus enfant.
**
**    [PROCESSUS ENFANT (pid == 0)] :
**    - Réinitialise SIGINT/SIGQUIT à leur comportement par défaut (`SIG_DFL`).
**    - `exec_node_in_child()` cherche dans le PATH et exécute `/bin/ls`.
**    - Si `exec` échoue, appelle `cleanup_and_exit(root, env, 127)`.
**
**    [PROCESSUS PARENT (pid > 0)] :
**    - `waitpid(...)` attend la fin du processus enfant.
**    - `handle_child_status(wstatus)` extrait le code de sortie normalisé.
**   
	- `setup_signal_handlers()` restaure les gestionnaires de signaux interactifs.
**    - Renvoie le code de statut.
*/
static int	need_right(t_node_type t, int status)
{
	if (t == N_AND)
		return (status == 0);
	return (status != 0);
}

int	run_tree(t_node *root, t_var **env, int last_status)
{
	int status;

	if (root->type == N_AND || root->type == N_OR)
	{
		status = run_tree(root->left, env, last_status);
		if (need_right(root->type, status))
			status = run_tree(root->right, env, status);
		return (status);
	}
	if (is_parent_builtin_root(root))
		return (run_builtin_in_parent(root, env, last_status));
	return (fork_and_run(root, env));
}