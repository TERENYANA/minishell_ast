# Minishell — Réponses de Défense Technique

Ce document contient les réponses aux questions de défense technique pour le projet Minishell, en se basant sur une architecture avec un Arbre Syntaxique Abstrait (AST).

## 1. Architecture et flux général

**1. Trace les données depuis l'entrée utilisateur jusqu'à l'affichage du résultat.**
Chaîne de caractères brute (readline) → Liste de tokens (Lexer) → AST (Parser) → Expansion des variables/wildcards (avant ou pendant l'exécution) → Exécution via parcours de l'AST.

**2. Pourquoi avoir choisi un AST plutôt qu'une liste plate de commandes ?**
Je fais un AST parce qu'il représente hiérarchiquement la grammaire (parenthèses, priorités des opérateurs `&&`, `||`, `|`). Si je faisais une liste plate, il serait très complexe de gérer correctement l'imbrication des priorités et l'exécution conditionnelle, car l'ordre d'évaluation ne serait pas naturel.

**3. Ton t_node est une seule structure pour les cinq types de nœuds. Pourquoi ?**
Je fais une structure unique parce que cela simplifie grandement la gestion de la mémoire (une seule fonction de free) et le parcours de l'arbre. Si je faisais 5 structures distinctes, il faudrait caster constamment les pointeurs ou utiliser des `union`, ce qui alourdirait le code C.

**4. Quel est le coût de cette décision d'utiliser une structure unique t_node ?**
Le coût est un léger gaspillage de mémoire : un nœud de type commande possède des pointeurs `left`/`right` inutiles, et un nœud opérateur possède un tableau `args` inutile.

**5. Pourquoi t_node possède-t-il exactement cinq champs ?**
Il contient généralement : le type du nœud, le tableau d'arguments (pour les commandes), les redirections, et les pointeurs `left` et `right` (pour les enfants). C'est le strict minimum pour représenter à la fois des commandes et des opérateurs binaires/unaires.

**6. Si tu devais ajouter un sixième champ à t_node, comment déterminerais-tu où le placer et pourquoi ?**
Je le placerais à la fin ou avec les pointeurs pour des raisons d'alignement mémoire (padding). Le but est de regrouper les types de données similaires (pointeurs ensemble, entiers ensemble) pour optimiser la taille de la structure.

**7. Où se trouve la priorité des opérateurs dans ton code ?**
Elle se trouve implicitement dans l'ordre d'appel des fonctions du parser (Descente Récursive). La règle la moins prioritaire est appelée en premier (ex: `parse_and_or`), et elle appelle les règles plus prioritaires (ex: `parse_pipeline`).

**8. Comment peux-tu montrer que cette priorité n'est pas définie par une table explicite ?**
En montrant le code du parser : il n'y a pas de tableau de priorités. C'est la structure des appels de fonctions (`parse_and_or` -> `parse_pipeline` -> `parse_command`) qui impose que la commande soit évaluée avant le pipeline, et le pipeline avant le `&&`/`||`.

## 2. Lexer et tokenisation

**9. Comment distingues-tu `>>` de deux `>` successifs ?**
Le lexer lit le caractère `>`. S'il voit un autre `>` juste après, il les groupe pour former un seul token `APPEND` (`>>`). S'il y a un espace ou autre chose, il forme un token `TRUNC` (`>`).

**10. Pourquoi l'ordre de détection de `>>` et `>` est-il important ?**
Je cherche `>>` avant `>` parce que si je cherchais d'abord `>`, je couperais `>>` en deux tokens `>` distincts, ce qui changerait totalement le sens (deux redirections simples au lieu d'un mode append).

**11. Pour `echo "a"b'c'`, combien obtiens-tu de tokens ?**
On obtient DEUX tokens : `echo` et `"a"b'c'`.

**12. Pourquoi `echo "a"b'c'` produit-il un seul argument au lieu de trois ?**
Parce que dans le shell, les mots sont séparés par des espaces non quotés. Ici, il n'y a pas d'espaces entre `"a"`, `b`, et `'c'`, donc ils forment un seul mot lexical (token), qui sera ensuite concaténé lors de l'expansion.

**13. Que fait exactement `is_special()` ?**
Elle vérifie si un caractère est un métacaractère du shell (ex: `|`, `<`, `>`, `(`, `)`, espace) qui doit couper le mot actuel.

**14. Pourquoi `is_special()` reconnaît-il exactement ces six caractères ?**
Car ce sont les opérateurs de contrôle et de redirection définis par POSIX (et le sujet). Ils ont un sens syntaxique propre, contrairement aux lettres.

**15. Que se passe-t-il lorsqu'une quote n'est pas fermée ?**
Le lexer arrive à la fin de la chaîne sans avoir trouvé la quote fermante. C'est une erreur de syntaxe.

**16. Que retourne le lexer dans ce cas ?**
Il retourne une erreur (souvent un code spécifique ou NULL) et libère les tokens déjà créés.

**17. Que voit l'appelant lorsque le lexer détecte une quote non fermée ?**
Il voit une erreur lexicale, n'appelle pas le parser, affiche un message d'erreur et met `$?` à 2 (ou autre code d'erreur de syntaxe).

**18. Pourquoi `assign_type()` regarde-t-il le premier caractère du token plutôt que l'intégralité du token ?**
Pour les opérateurs, le premier caractère (ex: `<`) suffit souvent pour savoir que c'est une redirection, puis on regarde la longueur pour distinguer `<` et `<<`. Cela évite un `ft_strncmp` sur toute la chaîne pour les cas évidents.

**19. Quels types de tokens peuvent être identifiés uniquement grâce à leur premier caractère ?**
Les parenthèses `(`, `)`, le pipe `|`, et souvent le début des redirections `<` et `>`.

## 3. Parser et grammaire

**20. Écris les trois règles principales de ta grammaire.**
- Liste (&&, ||) : `pipeline { ("&&" | "||") pipeline }`
- Pipeline (|) : `commande { "|" commande }`
- Commande : `(subshell | commande_simple) { redirections }`

**21. Quel niveau de la grammaire correspond à chaque priorité d'opérateur ?**
- Niveau bas (fortement lié) : Commande (mots)
- Niveau moyen : Pipeline (`|`)
- Niveau haut (faiblement lié) : Liste (`&&`, `||`)

**22. Comment la structure de ta grammaire encode-t-elle la priorité des opérateurs ?**
Plus un opérateur est prioritaire, plus il est traité profondément dans l'arbre d'appels récursifs, il se retrouve donc plus bas (plus proche des feuilles) dans l'AST, et sera exécuté en premier.

**23. Explique l'astuce `left = new_op_node(...)`.**
Lorsqu'on parse `a | b | c`, on parse d'abord `a` (left). En voyant `|`, on crée un nœud opérateur où `left` devient l'enfant gauche, et on parse `b` pour l'enfant droit. Ce nœud opérateur devient le nouveau `left` pour la suite, ce qui construit l'arbre en associativité à gauche.

**24. Dessine l'arbre construit pour `a | b | c`.**
```
      | (le 2e)
     / \
  | (1er) c
 / \
a   b
```

**25. Pourquoi le pipeline est-il associatif à gauche ?**
Pour que les données coulent logiquement de gauche à droite, et pour respecter la sémantique de Bash : le premier `|` lie `a` et `b`, puis le second `|` lie le résultat de `a|b` à `c`.

**26. Donne deux raisons justifiant cette associativité à gauche.**
- Le flux de données : `a` produit pour `b`, qui produit pour `c`.
- L'exécution : Il est plus simple d'évaluer de gauche à droite pour chainer les descripteurs de fichiers (pipes).

**27. Pour `a | b | c`, combien de fois `new_cmd_node()` est-il appelé ?**
3 fois (pour `a`, `b`, et `c`).

**28. À quel moment la racine de l'arbre apparaît-elle dans `a | b | c` ?**
Elle apparaît en dernier, lorsque le deuxième `|` est parsé. Le nœud racine est le pipe qui relie `a|b` et `c`.

**29. Qu'est-ce que l'« ascenseur » dans `parse_primary()` ?**
C'est le fait que lorsqu'on rencontre une parenthèse `(`, on "remonte" tout en haut de la grammaire en appelant `parse_list()` (ou `parse_and_or`), ce qui permet de réévaluer toute une expression complexe comme un seul bloc primaire.

**30. Pourquoi `(` appelle-t-il `parse_list()`, c'est-à-dire le niveau le plus haut de la grammaire ?**
Parce que le contenu d'un subshell peut être n'importe quelle commande complète, y compris des pipelines et des opérateurs logiques.

**31. Pourquoi `parse_command()` s'arrête-t-il lorsqu'il rencontre `PIPE` sans consommer le token ?**
Parce que `PIPE` ne fait pas partie de la syntaxe d'une commande simple. C'est à la fonction `parse_pipeline()` (qui a appelé `parse_command()`) de consommer ce pipe et de lier les commandes.

**32. Quelle fonction est responsable de consommer le `PIPE` ?**
`parse_pipeline()` (ou l'équivalent qui gère les pipes).

**33. Pourquoi `is_cmd_end()` contient-il `RPAREN` mais pas `LPAREN` ?**
Parce que `)` marque la fin d'une sous-expression valide, alors que `(` indique le début d'un subshell. Au milieu d'une commande simple, `(` est soit interdit (erreur), soit traité au début d'une primaire.

**34. Explique l'asymétrie entre `LPAREN` et `RPAREN`.**
`LPAREN` initie un nouveau contexte de parsing (descente), alors que `RPAREN` clôture un contexte et déclenche un retour (remontée).

**35. Que se passe-t-il si `parse_command()` rencontre `(` au milieu d'une commande ?**
C'est une erreur de syntaxe, car un subshell `(` doit être le début d'une commande, il ne peut pas se trouver après des arguments (ex: `echo ( ls )` est invalide dans bash standard sans le `$`).

**36. Déroule les trois branches possibles de la boucle de `parse_command()` dans ce cas.**
- Si redirection : parser la redirection.
- Si subshell `(` en début : parser le subshell via `parse_list()`.
- Si mot : l'ajouter aux arguments. S'il voit `(` après un mot, il retourne une erreur de syntaxe.

## 4. Expansion

**37. Pourquoi `ft_expand()` est-il appelé dans le parser plutôt que dans une phase séparée ?**
(Note : dans certaines architectures, il est appelé à l'exécution). Si on l'appelle dans le parser ou juste avant l'exécution, c'est pour s'assurer que l'arbre a d'abord délimité les commandes de manière sécurisée sans être trompé par le contenu des variables.

**38. Que réponds-tu à l'objection : « Pourquoi ne pas faire l'expansion en un passage séparé sur l'AST ? »**
C'est souvent ce qu'on fait ! L'expansion se fait au moment de l'exécution (ou juste avant de lancer la commande) pour que `$USER` contienne la valeur au moment où la commande est exécutée, surtout si une commande précédente a fait un `export USER=truc`.

**39. Compare `echo '$USER'` et `echo "$USER"`.**
Le premier affiche littéralement `$USER`. Le second affiche la valeur de la variable d'environnement (ex: `msl`).

**40. Pourquoi ces deux commandes ont-elles des comportements différents ?**
Les simples quotes (`'`) désactivent toutes les interprétations (tout est littéral). Les doubles quotes (`"`) conservent le sens littéral de tous les caractères, SAUF `$`, `\`, et parfois backtick, permettant l'expansion des variables.

**41. Où dans ton code la distinction entre quotes simples et doubles quotes est-elle prise en compte ?**
Dans la fonction d'expansion : on itère sur la chaîne, on garde un état (IN_SINGLE, IN_DOUBLE, unquoted). Si on trouve un `$` en unquoted ou IN_DOUBLE, on l'étend. En IN_SINGLE, on ne l'étend pas.

**42. Pourquoi ne peux-tu pas simplement supprimer toutes les quotes avant de faire l'expansion des variables ?**
Si je supprimais les quotes d'abord, `echo '$USER'` deviendrait `echo $USER`, et le programme afficherait la valeur au lieu du littéral `$USER`. Je perdrais l'information de protection.

**43. Donne un contre-exemple montrant pourquoi l'ordre des opérations est important.**
`export VAR="a   b"`. Si on enlève les quotes avant, `echo "$VAR"` devient `echo "a   b"`. Si on expan d'abord, on sait que `a   b` doit rester un seul argument car il était dans des doubles quotes.

**44. Que produit `echo $NOPE` si NOPE n'existe pas ?**
Il produit un argument vide qui sera supprimé (Word Splitting). Le résultat est une ligne vide (echo sans argument).

**45. Que produit `echo "$NOPE"` ?**
Il produit un argument constitué d'une chaîne vide `""`. `echo` reçoit un argument (la chaîne vide) et affiche une ligne vide, mais techniquement `argv` est différent.

**46. Pourquoi le nombre d'éléments dans `argv` peut-il être différent dans ces deux cas ?**
Sans quotes, la variable non définie disparaît complètement, donc `echo` a 1 argument (`argv[0] = "echo"`). Avec quotes, la chaîne vide est préservée, donc `echo` a 2 arguments (`argv[0] = "echo"`, `argv[1] = ""`).

**47. Où cette différence est-elle vérifiée ou construite dans ton code ?**
Pendant la phase de création de `argv` (Word Splitting) qui suit l'expansion. Les éléments vides non quotés sont ignorés, les éléments vides quotés sont gardés.

**48. Comment gères-tu `$?` ?**
C'est un cas particulier. Si on trouve `$?`, on le remplace par la conversion en string de notre variable globale (ou état) qui stocke le dernier code de retour.

**49. Que se passe-t-il lorsqu'un `$` est suivi d'un caractère qui ne constitue pas un nom de variable ?**
Si c'est un espace ou une quote, le `$` est traité comme un caractère littéral. S'il est suivi d'un caractère invalide pour un début de nom (ex: `$1` qui n'est pas géré), il est étendu en vide.

**50. Pourquoi `$$` ne devient-il pas le PID du shell ?**
Parce que le sujet de Minishell demande explicitement de ne gérer qu'un ensemble limité de variables d'environnement, et `$?`. Les variables spéciales comme `$$` ou `$!` ne sont pas exigées.

**51. Quelles limitations du sujet expliquent ce comportement ?**
Le sujet demande : "Expand variables ($) to their values" et "$? to the exit status of the most recently executed foreground pipeline". Il ne demande pas les paramètres spéciaux.

## 5. Wildcards / expansion de *

**52. Pourquoi `has_unquoted_star()` regarde-t-il le token brut plutôt que le token déjà développé ?**
Pour s'assurer que l'étoile a été tapée par l'utilisateur et non introduite par l'expansion d'une variable (ex: `export VAR="*"; echo "$VAR"` ne doit pas s'étendre).

**53. Pourquoi est-il important de savoir si le `*` était entre quotes avant de le développer ?**
Si on le développe avant, on ne peut pas distinguer `echo "*"` (qui doit afficher `*`) de `echo *` (qui doit lister les fichiers).

**54. Explique le fonctionnement récursif de `wc_match()`.**
On compare le pattern et le nom du fichier. S'ils matchent, on avance. Si on voit `*` dans le pattern, on a deux choix : avancer le pattern en ignorant le `*` (qui matche vide), ou avancer le string (le `*` consomme un caractère).

**55. Pourquoi une étoile possède-t-elle deux branches possibles dans l'algorithme de matching ?**
Parce que `*` peut représenter zéro caractère (on passe au caractère suivant du pattern) ou N caractères (on consomme un caractère du fichier et on reste sur le `*`).

**56. Quelle différence y a-t-il entre `echo *` et `echo .*` ?**
`*` ne correspond pas aux fichiers commençant par un point (fichiers cachés). `.*` correspond explicitement aux fichiers cachés.

**57. Où cette règle concernant les fichiers cachés apparaît-elle dans ton code ?**
Au début de `wc_match` : si le fichier commence par `.` et que le pattern ne commence pas par `.`, on retourne false directement.

**58. Que fait ton shell lorsque `*.zzz` ne correspond à aucun fichier ?**
Il conserve le mot littéralement `*.zzz`.

**59. Pourquoi conserves-tu le pattern plutôt que de générer une erreur ?**
C'est le comportement standard de Bash (avec `nullglob` désactivé par défaut). Si aucun fichier ne correspond, le mot reste inchangé.

**60. Pourquoi le résultat de `readdir()` doit-il être trié ?**
`readdir()` retourne les fichiers dans l'ordre du système de fichiers (ordre de création ou de table d'inode), qui est imprévisible. Bash les affiche par ordre alphanumérique.

**61. Que se passerait-il si tu ne triais pas les résultats du wildcard ?**
`echo *` afficherait les fichiers dans le désordre, et l'évaluation échouerait lors des corrections automatiques.

## 6. Exécution — processus

**62. Pourquoi faut-il faire `fork()` avant `execve()` ?**
Je fais `fork()` parce que `execve()` remplace l'image du processus appelant par le nouveau programme. Si je fais `execve()` directement, mon minishell disparaît.

**63. Que se passerait-il si tu faisais directement `execve()` dans le processus du shell ?**
Le shell exécuterait la commande (ex: `ls`) puis le processus se terminerait. Le shell se fermerait à la première commande.

**64. Que retourne `fork()` dans le parent ?**
Il retourne le PID (Process ID) de l'enfant qui vient d'être créé.

**65. Que retourne `fork()` dans l'enfant ?**
Il retourne `0`.

**66. Comment ton code sait-il s'il se trouve dans le parent ou dans l'enfant ?**
En vérifiant la valeur de retour de `fork()` : `if (pid == 0)` on est l'enfant, `if (pid > 0)` on est le parent.

**67. Pourquoi utilises-tu `execve()` plutôt que `execvp()` ?**
Le sujet interdit `execvp()`. Nous devons recréer nous-mêmes la résolution du chemin via la variable d'environnement `PATH`.

**68. Donne deux raisons liées à ton implémentation.**
- L'interdiction par le sujet (seules certaines fonctions sont autorisées).
- Cela nous oblige à maîtriser la variable `$PATH` et la recherche d'exécutables (concaténation, `access()`).

**69. `execve()` est-il une fonction de bibliothèque ou un appel système ?**
C'est un appel système (syscall).

**70. Comment peux-tu le vérifier concrètement ?**
Dans le manuel : `man 2 execve` (la section 2 est réservée aux appels système).

**71. Que dois-tu passer comme troisième argument à `execve()` ?**
L'environnement sous forme d'un tableau de chaînes de caractères (type `char **`).

**72. Dans quel format cet argument doit-il être fourni ?**
Un tableau de chaînes, terminé par un pointeur `NULL`. Chaque chaîne est de la forme `CLE=VALEUR`.

**73. Pourquoi ne peux-tu pas passer directement ta liste `t_var` à `execve()` ?**
Ma liste `t_var` est une liste chaînée (ou un dictionnaire). `execve()` a besoin d'un format en C strict : un `char **` contigu en mémoire. Je dois donc la convertir avant l'appel.

**74. Pourquoi tout code exécuté après un `execve()` réussi est-il un signe d'erreur ?**
Parce qu'un `execve()` réussi ne retourne jamais : il remplace tout le programme. Si la ligne suivante est atteinte, c'est que `execve()` a échoué.

**75. Pourquoi le parent doit-il appeler `waitpid()` ?**
Pour attendre que l'enfant finisse son exécution, récupérer son code de retour, et nettoyer ses ressources du système.

**76. Donne quatre raisons justifiant l'utilisation de `waitpid()`.**
- Récupérer le code de sortie (`$?`).
- Empêcher l'enfant de devenir un "zombie".
- Bloquer le parent pour ne pas réafficher le prompt avant la fin de la commande.
- Pouvoir attendre un processus spécifique (important dans les pipelines).

**77. Qu'est-ce qu'un processus zombie ?**
Un processus qui a terminé son exécution, mais dont le parent n'a pas encore lu le code de retour via `wait()`. Il occupe une place dans la table des processus du noyau.

**78. Comment `waitpid()` permet-il d'éviter les zombies ?**
Il lit le statut de sortie de l'enfant, ce qui informe le noyau qu'il peut libérer complètement l'entrée de l'enfant dans la table des processus.

## 7. Exécution — redirections et pipes

**79. Comment fonctionne `dup2()` lors d'une redirection ?**
`dup2(oldfd, newfd)` copie le descripteur `oldfd` sur `newfd` (ex: `1` pour stdout). Ainsi, tout ce qui écrit sur `1` ira désormais dans le fichier pointé par `oldfd`.

**80. Pourquoi fermes-tu `fd` immédiatement après `dup2()` ?**
Parce que `fd` a été dupliqué vers `0` ou `1`. Le processus a maintenant deux descripteurs pointant vers le même fichier. On ferme l'original (`fd`) pour ne pas gaspiller de descripteurs (fuites de fd).

**81. Analyse `echo hi > a > b`.**
Le shell ouvre `a` en écriture, le lie à fd 1, puis ouvre `b` en écriture, et le lie à fd 1 (écrasant la liaison avec `a`).

**82. Pourquoi les deux fichiers sont-ils créés mais seul `b` reçoit `hi` ?**
L'ouverture (et tronquage) de `a` se produit en premier. Puis `b` est ouvert. Finalement, la commande s'exécute et sa sortie standard pointe vers `b`. Le fichier `a` est créé mais reste vide.

**83. Où dans ton code l'ordre des redirections est-il respecté ?**
En traitant la liste chaînée des redirections du nœud commande dans l'ordre (de gauche à droite).

**84. Déroule complètement l'exécution de `ls | wc -l`.**
- Création d'un pipe (`fd[0]`, `fd[1]`).
- Fork pour `ls` (enfant 1).
- Fork pour `wc` (enfant 2).
- Enfant 1 : `dup2(fd[1], STDOUT_FILENO)`, `close` des deux fd, `execve(ls)`.
- Enfant 2 : `dup2(fd[0], STDIN_FILENO)`, `close` des deux fd, `execve(wc)`.
- Parent : `close` des deux fd du pipe, `waitpid` pour enfant 1 et enfant 2.

**85. Combien de processus sont créés ?**
Deux processus enfants sont créés (un pour `ls`, un pour `wc`), plus le shell parent.

**86. Combien de pipes sont nécessaires ?**
Un seul pipe (qui contient une entrée `fd[1]` et une sortie `fd[0]`).

**87. Qui ferme quelle extrémité de chaque pipe ?**
- L'enfant 1 (ls) ferme `fd[0]` (lecture) et utilise `fd[1]` (écriture).
- L'enfant 2 (wc) ferme `fd[1]` (écriture) et utilise `fd[0]` (lecture).
- Le parent ferme les deux extrémités `fd[0]` et `fd[1]`.

**88. Pourquoi faut-il fermer les deux extrémités du pipe dans le parent ?**
Parce que le parent n'utilise pas le pipe. S'il les laisse ouverts, l'extrémité écriture ne sera jamais totalement fermée.

**89. Que se passe-t-il si le parent garde ouvert le côté écriture ?**
`wc -l` lira sur son entrée standard et attendra que toutes les extrémités en écriture soient fermées pour recevoir le signal EOF (End Of File). Puisque le parent le garde ouvert, `wc` attendra indéfiniment.

**90. Pourquoi `wc` peut-il rester bloqué dans `cat | wc` si l'extrémité écriture n'est jamais fermée ?**
`wc` attend EOF. Le noyau n'enverra EOF sur le pipe que lorsque TOUS les processus possédant un descripteur d'écriture sur ce pipe l'auront fermé.

**91. Quelle condition permet à `wc` de savoir qu'il a atteint la fin de l'entrée ?**
Un `read()` sur le pipe retourne 0, ce qui se produit quand tous les descripteurs d'écriture liés à ce pipe sont fermés.

## 8. Builtins et les deux contextes d'exécution

**92. Pourquoi `cd` doit-il s'exécuter dans le parent alors que `ls` peut s'exécuter dans l'enfant ?**
`cd` modifie le répertoire courant de travail du processus. Si je fais `fork()` et `cd` dans l'enfant, seul l'enfant changera de dossier. En mourant, le shell parent sera resté dans l'ancien dossier.

**93. Quels builtins doivent obligatoirement être exécutés dans le parent ?**
`cd`, `export`, `unset`, `exit`.

**94. Pourquoi exactement ces builtins ont-ils besoin du processus parent ?**
Car ils modifient l'état interne du shell : `cd` modifie le cwd, `export`/`unset` modifient les variables d'environnement, `exit` arrête le shell. Une modification dans un enfant est détruite à la mort de l'enfant.

**95. Que se passe-t-il avec `export X=1 | cat` ?**
Dans un pipeline, Bash crée un sous-shell (enfant) pour chaque commande. Donc `export X=1` s'exécute dans un enfant.

**96. Est-ce que `X` persiste après le pipeline ?**
Non. `X` est créé dans l'enfant puis disparaît avec lui.

**97. Pourquoi une modification de l'environnement réalisée dans l'enfant ne remonte-t-elle pas au parent ?**
Parce qu'au moment du `fork()`, l'enfant reçoit une COPIE de la mémoire du parent. Les modifications sur cette copie n'affectent pas la mémoire originale du parent (isolation des processus).

**98. Comment Bash se comporte-t-il dans ce cas ?**
Bash exécute chaque partie du pipe dans un sous-shell, donc `export` dans un pipe n'affecte pas l'environnement du parent.

**99. Comment ton code détermine-t-il qu'une commande est un builtin devant être exécuté dans le parent ?**
Je vérifie si le nom de la commande correspond à un builtin. Si elle n'est pas dans un pipeline ou un sous-shell (donc exécution seule), elle s'exécute dans le parent sans `fork()`.

**100. Quelles sont les trois conditions nécessaires pour considérer une commande comme un builtin-parent ?**
- C'est un builtin modifiant l'état (cd, export, unset, exit).
- Il est exécuté comme commande simple (pas de `|`).
- Il n'est pas encapsulé dans des parenthèses `(subshell)`.

## 9. Subshell — N_SUB

**101. À quoi sert `N_SUB` dans ton AST ?**
À représenter un bloc entouré de parenthèses `( ... )`.

**102. `N_SUB` sert-il à gérer `&&` et `||` ?**
Non, `&&` et `||` ont leurs propres nœuds (`N_AND`, `N_OR`).

**103. À quoi sert-il réellement ?**
Il sert à isoler l'exécution d'un bloc de commandes dans un sous-shell. Toute modification de l'environnement (cd, export) à l'intérieur ne fuira pas à l'extérieur.

**104. En quoi `N_SUB` diffère-t-il structurellement des autres types de nœuds ?**
Il possède un seul enfant (souvent `left`) qui pointe vers un arbre de commandes, et il peut posséder ses propres redirections (ex: `(ls; cd /) > out`).

**105. Où le contenu du subshell est-il stocké ?**
Dans le pointeur `left` du nœud `N_SUB`.

**106. Compare `cd /tmp` et `(cd /tmp)`.**
Le premier modifie le répertoire du shell courant. Le second forke, change de dossier dans l'enfant, puis l'enfant meurt, laissant le parent dans son dossier initial.

**107. Pourquoi leur effet sur le répertoire courant est-il différent ?**
À cause de l'isolation par processus : `(` force le forking d'un sous-shell.

**108. Pourquoi aucun `fork()` spécifique à `N_SUB` n'est-il écrit directement dans le parser ?**
Parce que le parser ne fait que construire l'arbre (analyse syntaxique). L'exécution (et donc les fork) se fait pendant la phase de parcours de l'arbre.

**109. D'où vient l'isolation du subshell ?**
Elle vient du fait qu'au moment de l'exécution, si l'exécuteur rencontre `N_SUB`, il appelle `fork()` et exécute la branche dans le processus enfant.

**110. Pour `(echo hi) > out`, où doit être attachée la redirection ?**
La redirection `> out` est attachée au nœud `N_SUB` lui-même.

**111. Pourquoi la redirection appartient-elle au groupe plutôt qu'à `echo` directement ?**
Parce que les parenthèses groupent les commandes. La redirection s'applique à la sortie de l'intégralité du bloc, pas seulement de la première commande.

## 10. Signaux — partie critique

**112. Pourquoi utilises-tu `sigaction()` pour gérer Ctrl-C ?**
Parce que `sigaction()` est recommandé par POSIX, permet de vider le masque de signaux et d'éviter que le signal ne soit bloqué par inadvertance. Il offre plus de contrôle que l'ancien `signal()`.

**113. Pourquoi Ctrl-D n'utilise-t-il pas `sigaction()` ?**
Parce que Ctrl-D n'envoie pas de signal.

**114. Ctrl-D est-il un signal ?**
Non.

**115. Si ce n'est pas un signal, qu'est-ce que Ctrl-D représente ?**
Il représente le caractère EOF (End Of File) pour le terminal.

**116. Que retourne `read()` lorsqu'il rencontre EOF sur le terminal ?**
Il retourne `0` octets lus.

**117. Que retourne `readline()` dans ce cas ?**
Il retourne le pointeur `NULL`.

**118. Comment ton code détecte-t-il Ctrl-D ?**
En vérifiant la valeur de retour de `readline()`. Si `line == NULL`, cela signifie EOF, et je quitte le shell.

**119. Explique précisément l'asymétrie Ctrl-C / Ctrl-D.**
Ctrl-C déclenche un signal (SIGINT) intercepté de manière asynchrone par un handler. Ctrl-D est une fin de flux de données gérée de manière synchrone par la fonction de lecture.

**120. Pourquoi utiliser `sigaction()` plutôt que `signal()` pour SIGINT ?**
`signal()` a un comportement non standardisé selon les UNIX (ex: réinitialisation du handler ou non). `sigaction()` garantit la portabilité et permet de contrôler précisément le comportement via `sa_flags`.

**121. Qu'apporte le contrôle explicite des flags avec `sigaction()` ?**
Il permet d'activer ou non des comportements spécifiques, par exemple interdire `SA_RESTART` pour s'assurer que `readline()` est bien interrompu.

**122. Qu'est-ce que `SA_RESTART` ?**
C'est un flag qui dit au noyau : "Si un appel système (comme `read()`) est interrompu par ce signal, relance-le automatiquement une fois le handler terminé".

**123. Pourquoi ne l'utilises-tu pas ?**
Parce que dans Minishell, si on fait Ctrl-C, on VEUT interrompre `readline()` pour afficher un nouveau prompt (via `rl_on_new_line`, etc.). Si je mettais `SA_RESTART`, `readline()` resterait bloqué à attendre.

**124. Quel rapport y a-t-il entre `SA_RESTART` et `readline()` ?**
`readline()` utilise `read()`. Sans `SA_RESTART`, le signal interrompt `read()`.

**125. Pourquoi fais-tu `sigemptyset(&sa.sa_mask)` ?**
Pour s'assurer qu'aucun autre signal ne sera bloqué pendant l'exécution de mon handler.

**126. Que pourrait-il se passer si le masque contenait des valeurs non initialisées ?**
Des signaux arbitraires pourraient être bloqués de manière imprévisible pendant la réception d'un Ctrl-C, provoquant des bugs subtils.

**127. Pourquoi ton programme possède-t-il une seule variable globale ?**
Parce que le sujet de Minishell autorise au maximum UNE variable globale, destinée exclusivement à stocker le numéro du signal reçu.

**128. Pourquoi cette variable globale ne contient-elle que le numéro du signal ?**
Pour respecter la contrainte stricte du sujet et parce qu'un handler de signal ne peut communiquer proprement avec le programme principal que via une variable globale ou un pipe.

**129. Qu'est-ce que le sujet interdit concernant les variables globales ?**
Il interdit d'utiliser des variables globales pour stocker l'AST, l'environnement, ou des structures compliquées.

**130. Pourquoi la variable est-elle de type `volatile sig_atomic_t` ?**
C'est le seul type garanti par le standard C pour être lu et écrit en une seule instruction atomique de manière sûre, sans être optimisé par le compilateur.

**131. Pourquoi `volatile` est-il important pour une variable modifiée dans un handler ?**
Il indique au compilateur que la valeur peut changer à tout moment de manière externe au flux normal. Cela l'empêche de mettre la variable en cache dans un registre.

**132. Pourquoi utiliser `sig_atomic_t` plutôt qu'un `int` classique ?**
Sur certains systèmes anciens ou exotiques, lire ou écrire un `int` peut nécessiter deux instructions machines. Si le signal intervient entre les deux, on lit une valeur corrompue. `sig_atomic_t` évite ça.

## 11. Signaux autour de fork()

**133. Quels sont les trois contextes de gestion des signaux autour d'un `fork()` ?**
1) Le shell interactif (parent). 2) L'enfant exécutant une commande (par défaut). 3) Le parent pendant qu'il attend l'enfant avec `waitpid()`.

**134. Que fait le parent avant le `fork()` ?**
Il ignore `SIGQUIT` et capte `SIGINT` pour réafficher le prompt.

**135. Que fait l'enfant immédiatement après le `fork()` ?**
Il restaure les actions par défaut pour `SIGINT` et `SIGQUIT` (avec `SIG_DFL`) pour qu'un Ctrl-C ou Ctrl-\ tue bien la commande en cours.

**136. Que fait le parent après `waitpid()` ?**
Il remet ses handlers interactifs pour pouvoir capter à nouveau Ctrl-C sur son propre prompt. Pendant le `waitpid`, il ignore souvent `SIGINT` (car l'enfant s'en occupe et meurt, et le parent mettra à jour `$?`).

**137. Pourquoi ces comportements doivent-ils être différents ?**
Parce que Ctrl-C dans le prompt doit juste faire une nouvelle ligne. Ctrl-C pendant `cat` doit tuer `cat` et le shell parent ne doit pas mourir, juste reprendre la main.

**138. Pourquoi le heredoc est-il forké ?**
Parce que `readline()` dans un heredoc est bloquant. En le forquant, si on fait Ctrl-C, on peut tuer l'enfant du heredoc sans interrompre tout le parsing du shell principal.

**139. Pourquoi ne pas simplement faire `close(STDIN)` dans le handler du heredoc ?**
Parce que la fonction `close()` n'est pas toujours async-signal-safe, et manipuler les fd dans un handler est dangereux. De plus, cela polluerait l'état de STDIN pour le reste du shell.

**140. Comment l'enfant du heredoc informe-t-il le parent qu'il a été interrompu par Ctrl-C ?**
Il exit avec un code de retour spécifique (souvent 130).

**141. Quel code de sortie utilise-t-il ?**
130 (128 + numéro de SIGINT).

**142. Comment le parent vérifie-t-il cette interruption ?**
En appelant `waitpid()` sur l'enfant du heredoc et en vérifiant si le code de sortie vaut 130. Si oui, il arrête tout le parsing.

**143. Pourquoi un handler de signal doit-il être le plus court possible ?**
Parce qu'il interrompt l'exécution asynchrone du programme. Plus il est long, plus le risque d'inter-blocage ou de corruption de données partagées est grand.

**144. Qu'est-ce qu'une fonction async-signal-safe ?**
C'est une fonction qui peut être appelée sans danger depuis un handler de signal (ex: `write`, mais PAS `printf` ou `malloc`).

**145. Pourquoi est-il dangereux d'appeler des fonctions arbitraires depuis un handler ?**
Si on appelle `malloc` et que le signal a interrompu un autre `malloc`, le mutex interne de la libc est déjà verrouillé. Le handler va essayer de le verrouiller et provoquer un deadlock (blocage définitif).

**146. Pourquoi Ctrl-\ (SIGQUIT) ne fait-il généralement rien en mode interactif ?**
Parce que le shell parent ignore explicitement `SIGQUIT` (avec `SIG_IGN`) pour éviter de quitter le shell en générant un coredump par accident.

**147. Pourquoi SIGQUIT peut-il malgré tout tuer une commande en cours d'exécution ?**
Parce qu'au moment du `fork()`, l'enfant restaure `SIGQUIT` à son comportement par défaut (`SIG_DFL`).

## 12. Codes de sortie

**148. D'où vient la valeur de `$?` ?**
Elle vient du code de retour du dernier processus exécuté, récupéré via `waitpid()`.

**149. Comment cette valeur voyage-t-elle du processus enfant jusqu'au shell ?**
L'enfant fait `exit(code)`. Le noyau stocke ce code. Le parent appelle `waitpid(pid, &wstatus, 0)` et le noyau écrit le code dans `wstatus`.

**150. Qu'est-ce que `wstatus` retourné par `waitpid()` ?**
C'est un entier contenant des informations encodées sur la manière dont l'enfant est mort (code de sortie normal, ou mort par un signal).

**151. Pourquoi `wstatus` n'est-il pas directement le code de retour de la commande ?**
Parce qu'il utilise différents bits pour stocker d'autres infos : savoir s'il a été tué par un signal, s'il a core dump, etc.

**152. Quelles macros permettent d'interpréter `wstatus` ?**
`WIFEXITED(wstatus)`, `WEXITSTATUS(wstatus)`, `WIFSIGNALED(wstatus)`, `WTERMSIG(wstatus)`.

**153. D'où vient la formule 128 + numéro_du_signal ?**
C'est la convention standard de Bash (et POSIX). Si un processus meurt du signal 2 (SIGINT), le shell fixe `$?` à 128 + 2 = 130.

**154. Pourquoi utilise-t-on 128 dans cette convention ?**
Parce que les codes de retour normaux (`exit()`) vont de 0 à 127. Au-delà, le bit 128 est utilisé pour signaler qu'une terminaison anormale (par signal) a eu lieu.

**155. Pourquoi une commande introuvable retourne-t-elle généralement 127 ?**
C'est une convention POSIX / Bash. Si le shell ne trouve pas l'exécutable dans le `$PATH`, il simule une erreur et retourne 127.

**156. Pourquoi une commande trouvée mais non exécutable retourne-t-elle 126 ?**
C'est aussi une convention. L'exécutable existe (le chemin est bon) mais on n'a pas les droits d'exécution (Permission denied). On retourne 126.

**157. D'où viennent ces deux valeurs ?**
Du standard POSIX qui définit le comportement d'un interpréteur de commandes.

**158. Pourquoi fais-tu un cast en `(unsigned char)` avant `exit()` ?**
`exit()` ne prend en compte que les 8 bits de poids faible de l'entier. Le comportement est implicitement un modulo 256.

**159. Que se passe-t-il si tu fais `exit(256)` ?**
`256 % 256 = 0`. Le shell parent lira un code de retour de 0 (succès), ce qui est un bug si on voulait signaler une erreur.

**160. Pourquoi le code de sortie d'un processus est-il limité sur les systèmes Unix classiques ?**
Parce que la structure d'attente historique stocke le code de retour sur un octet (8 bits, soit de 0 à 255) dans l'entier `wstatus`.

## 13. Mémoire et nettoyage

**161. Pourquoi `ft_free_node()` est-elle récursive ?**
Parce que l'AST est une structure d'arbre récursive. Pour libérer la racine, il faut libérer la branche gauche, la branche droite, puis la racine.

**162. Déroule `ft_free_node()` sur un arbre contenant quatre commandes.**
Le parcours est de type "post-order" (postfixé). Il descend tout à gauche, libère, remonte, va à droite, libère, remonte, etc., avant de libérer le parent.

**163. Dans quel ordre les nœuds sont-ils libérés ?**
De bas en haut (post-order). Les enfants sont toujours libérés avant leur nœud parent.

**164. Que doit libérer un nœud opérateur ?**
Ses pointeurs `left` et `right`. (Et le nœud lui-même).

**165. Que doit libérer un nœud commande ?**
Ses arguments (le `char **argv` et toutes les strings à l'intérieur) et sa liste de redirections. (Et le nœud lui-même).

**166. Pourquoi `add_arg()` libère-t-elle `value` elle-même en cas d'échec ?**
Pour éviter une fuite. Si elle n'arrive pas à allouer de la place dans le tableau pour stocker `value`, elle doit détruire `value` car l'appelant s'attend à ce que `add_arg` en prenne la responsabilité.

**167. Qu'est-ce que le « contrat de propriété » d'un pointeur ?**
C'est une règle logique qui dit : "Quelle fonction est responsable de faire free() sur cette allocation ?".

**168. À partir de quel moment `add_arg()` devient-elle propriétaire de `value` ?**
Dès qu'elle la reçoit en paramètre, elle assume qu'elle doit l'insérer ou la détruire en cas de problème d'allocation interne.

**169. Pourquoi `new_op_node()` doit-elle libérer ses deux enfants en cas d'échec ?**
Si `malloc` échoue pour créer le nœud opérateur, les enfants qui lui étaient destinés et qui avaient déjà été créés se retrouveraient orphelins, entraînant une fuite de mémoire.

**170. Que risque-t-on si elle ne le fait pas ?**
Une fuite (memory leak).

**171. Pourquoi `cleanup_and_exit()` libère-t-elle la mémoire dans un processus qui va mourir immédiatement ?**
Pour que Valgrind soit content ! 

**172. Est-ce réellement nécessaire techniquement ?**
Non. Sur un OS moderne, quand un processus meurt (`exit()`), le noyau réclame absolument toutes ses pages mémoire. Il n'y a techniquement aucune vraie fuite sur le système.

**173. Pourquoi est-ce néanmoins une bonne pratique dans ton projet ?**
Pour valider le projet à 42 : Valgrind doit marquer "All heap blocks were freed -- no leaks are possible". C'est un gage de rigueur.

**174. Que signifie `--trace-children=yes` avec Valgrind ?**
Cela demande à Valgrind d'inspecter non seulement le processus shell principal, mais aussi tous les processus qu'il crée via `fork()`.

**175. Pourquoi est-il obligatoire pour tester correctement Minishell ?**
Parce que l'essentiel du code d'exécution tourne dans les enfants (après `fork`). Si un enfant fuite de la mémoire et qu'on ne trace pas les enfants, Valgrind ne dira rien.

**176. Pourquoi les processus enfants peuvent-ils sinon cacher des fuites mémoire ?**
Sans cette option, Valgrind s'arrête au `fork()`. L'enfant hérite de l'environnement, crée des tableaux, et fait `exit`. La fuite dans l'enfant est invisible pour le parent.

**177. Où `heredoc_fd` est-il fermé ?**
Il est fermé après avoir été utilisé (souvent juste après le `dup2` ou à la fin de l'exécution de la commande concernée) et lors du nettoyage complet.

**178. Pourquoi est-ce le seul file descriptor possédé directement par une structure ?**
Parce que contrairement aux autres fichiers qui sont ouverts juste avant l'exécution, le heredoc est lu *pendant le parsing*. On crée un pipe/fichier temporaire qu'on doit garder ouvert dans l'arbre en attendant l'exécution.

## 14. Pièges et compréhension approfondie

**179. Qu'est-ce qui survit à `execve()` ?**
Le PID, le PPID, et les descripteurs de fichiers (file descriptors) ouverts, à moins qu'ils n'aient le flag `O_CLOEXEC`.

**180. Qu'est-ce qui est remplacé ou effacé par `execve()` ?**
Tout l'espace mémoire : le code (text), les variables globales (data/BSS), le tas (heap), et la pile (stack).

**181. Pourquoi les descripteurs de fichiers survivent-ils à `execve()` ?**
Parce qu'ils appartiennent à la structure du processus dans le noyau, pas à la mémoire de l'application utilisateur.

**182. Quel rapport direct y a-t-il entre cette propriété et les redirections ?**
C'est ce qui PERMET les redirections ! On fait `dup2()` (qui modifie les fd du noyau), PUIS `execve()`. Le nouveau programme (ex: `ls`) se réveille, écrit bêtement sur fd 1, sans savoir que fd 1 pointe vers un fichier au lieu de l'écran.

**183. Pourquoi fais-tu `dup2()` avant `execve()` ?**
Parce qu'après `execve()`, mon code n'existe plus. Je ne pourrais plus exécuter `dup2()`. Je dois préparer la plomberie avant de lancer l'eau (le nouveau programme).

**184. Pourquoi `t_parse_info` est-il passé par pointeur plutôt que par valeur ?**
Parce que c'est une structure qui contient l'état de l'itération (l'index du token actuel). Les fonctions récursives (descente récursive) doivent toutes avancer cet index commun.

**185. Que se passerait-il si une fonction imbriquée modifiait `t_parse_info` alors qu'il était passé par valeur ?**
Elle modifierait sa propre copie locale. En retournant, la fonction appelante continuerait à lire les anciens tokens. Boucle infinie ou erreur de syntaxe garantie.

**186. Pourquoi `env` est-il un simple pointeur alors que `error_code` est un double pointeur ?**
(Dépend de l'implémentation). Souvent, si on passe un paramètre pour en modifier le pointeur lui-même dans la fonction appelante (ex: changer ce vers quoi pointe `env`), il faut un `**`. Si c'est juste un int à modifier, un `*` suffit.

**187. Quelle règle générale permet de décider entre `T *` et `T **` ?**
Si je veux modifier la chose pointée, j'envoie un pointeur (`T *`). Si je veux changer L'ADRESSE du pointeur lui-même (réallouer ou rediriger), j'envoie l'adresse de mon pointeur (`T **`).

**188. Dans `false && echo x`, pourquoi `echo x` ne s'exécute-t-il pas ?**
À cause de l'exécution paresseuse (short-circuit evaluation). `false` retourne 1 (erreur). Le `&&` exige que le côté gauche soit un succès (0) pour exécuter le côté droit.

**189. Où cette exécution paresseuse est-elle représentée dans l'AST ?**
Lors du parcours du nœud `N_AND` :
`if (execute(node->left) == 0)`
`    execute(node->right);`

**190. Qu'est-ce qui détermine le `$?` final d'un pipeline `a | b | c` ?**
La commande la plus à droite du pipeline (ici `c`).

**191. Pourquoi la dernière commande du pipeline détermine-t-elle le résultat final ?**
C'est le comportement standard POSIX. Le statut du pipeline entier est le statut de sortie de la dernière commande, car le but du pipeline est le résultat final à droite.

## 15. Limites du projet et choix d'architecture

**192. S'il fallait ajouter des boucles `for`, qu'est-ce qu'il faudrait modifier dans l'architecture ?**
Il faudrait ajouter un nouveau nœud `N_FOR` à l'AST et modifier la grammaire du parser pour reconnaître `for X in Y; do Z; done`.

**193. Pourquoi un simple ajout dans le lexer ne suffirait-il pas ?**
Parce qu'une boucle `for` définit un bloc d'exécution différé et répétitif. Le parser doit construire l'arbre (condition, corps) pour que l'exécuteur puisse parcourir le corps plusieurs fois sans repasser par le texte source.

**194. Quelles nouvelles structures ou quels nouveaux types de nœuds seraient nécessaires ?**
`N_FOR`, `N_WHILE`. Un nœud avec au moins 3 informations : la variable d'itération, la liste de mots (ou condition), et le nœud enfant pour le bloc de code.

**195. Pourquoi `echo hello\ world` affiche-t-il littéralement le comportement prévu par ton projet ?**
Dans bash, `\` échappe l'espace. Si l'implémentation de minishell n'a pas inclus `\` (le sujet demande spécifiquement les quotes mais souvent interdit/ignore `\`), ça devient un caractère non géré et peut s'afficher littéralement (ou erreur selon le choix).

**196. Est-ce un bug ou une conformité au sujet ?**
C'est une conformité au sujet (souvent le backslash n'est pas demandé, "unclosed quotes... Not required to handle '\' (backslash)" dans certaines versions du sujet).

**197. Quels caractères spéciaux sont explicitement exclus par le sujet ?**
`\`, `;` (souvent), et la gestion des paramètres non clôturés ou des accolades.

**198. Pourquoi cette limitation simplifie-t-elle le lexer ?**
On n'a pas à gérer un état d'échappement complexe caractère par caractère, il suffit de toggle le booléen `in_quote` en voyant `'` ou `"`.

**199. Quelles fonctionnalités de Bash complètes ne sont volontairement pas représentées dans ton architecture ?**
Les jobs en arrière-plan (`&`), les fonctions, les boucles, les tableaux (arrays).

**200. Quelle est, selon toi, la principale limite de ton Minishell par rapport à Bash ?**
L'absence de syntaxe complexe (if, while) et la gestion des processus en arrière-plan et de contrôle des tâches (`fg`, `bg`).

## 16. Questions « pièges » de l'examinateur

**201. Si je supprime `fork()`, quelle partie de Minishell cesse immédiatement de fonctionner correctement ?**
L'exécution des commandes externes (elles termineraient le shell) et les pipes (qui nécessitent du parallélisme).

**202. Si je remplace `sigaction()` par `signal()`, quel contrôle est-ce que je perds ?**
Le contrôle sur la relance automatique des appels système via l'absence de flag équivalent à la désactivation de `SA_RESTART` de manière portable, et le contrôle du masque pendant l'exécution du handler.

**203. Si j'ajoute `SA_RESTART`, quel comportement de Ctrl-C peut changer ?**
Le Ctrl-C n'interromprait plus `readline()`. Rien ne s'afficherait jusqu'à ce qu'on appuie sur Entrée.

**204. Si je ne ferme pas le côté écriture d'un pipe dans le parent, quel processus risque de rester bloqué ?**
Le processus lisant le pipe (celui à droite du `|`, ex: `wc`), bloqué dans un `read()` attendant EOF.

**205. Si je fais `cd` dans l'enfant, pourquoi le répertoire courant du shell ne change-t-il pas ?**
Je fais `cd` dans l'enfant parce que c'est une erreur d'architecture. Si je le faisais dans le parent, il changerait, mais dans l'enfant, la modification de l'environnement disparaît à la mort de l'enfant (process isolation).

**206. Si `export` est exécuté dans un pipeline, pourquoi sa modification ne revient-elle pas au shell parent ?**
Parce que les pipelines impliquent l'exécution de chaque commande dans un sous-processus fork. Le parent ne voit pas les modifs du tas de ses enfants.

**207. Si `execve()` réussit, pourquoi le code qui suit ne doit-il normalement jamais être exécuté ?**
Parce qu'`execve()` n'est pas un appel de fonction classique, c'est un remplacement intégral du processus.

**208. Si `waitpid()` n'est jamais appelé, quels problèmes peuvent apparaître ?**
Les enfants deviennent des zombies (encombrent la table des PIDs), et le prompt s'affiche avant la fin de l'exécution de la commande.

**209. Si tu développes `$USER` avant de gérer les quotes, quel comportement incorrect obtiens-tu ?**
`echo '$USER'` affichera le contenu de la variable au lieu de la chaîne littérale `$USER`.

**210. Si tu retires les quotes avant de chercher les `*`, quels problèmes peux-tu créer ?**
`echo "*"` cherchera et listera des fichiers, alors qu'il devrait simplement afficher l'astérisque.

**211. Si tu ne tries pas les résultats de `readdir()`, quelle différence observable peux-tu avoir ?**
La liste des fichiers lors d'une expansion `*` s'affichera dans un ordre pseudo-aléatoire au lieu d'un ordre alphabétique.

**212. Si `*.zzz` ne correspond à rien, pourquoi ne dois-tu pas forcément retourner une erreur ?**
Parce que Bash sans option spécifique garde le pattern non altéré et le passe comme tel à la commande (qui elle, peut afficher une erreur).

**213. Si tu fais `echo "a"b'c'`, pourquoi les différentes quotes ne créent-elles pas plusieurs arguments ?**
Parce que l'espace est le délimiteur de mots. L'absence d'espace relie lexicalement les morceaux d'un même mot.

**214. Pourquoi un AST permet-il de représenter naturellement `(echo hi) > out`, alors qu'une simple liste de tokens rend l'exécution plus difficile ?**
Parce que dans un AST, `(echo hi)` devient un sous-arbre, et `> out` s'attache à la racine de ce sous-arbre. Dans une liste, l'exécuteur devrait chercher la parenthèse fermante, couper la liste, gérer des états internes de profondeur, ce qui est l'équivalent de refaire un AST à l'exécution.

**215. Si tu devais expliquer ton projet en une seule phrase à un examinateur, comment expliquerais-tu le rôle de l'AST ?**
L'AST transforme une suite linéaire de mots en un arbre de décisions hiérarchique qui dicte exactement comment et dans quel ordre exécuter, grouper et rediriger les commandes.

## 17. Les 15 questions à maîtriser absolument

**216. Explique précisément le chemin entrée → lexer → parser → AST → expansion → exécution.**
*(Voir Question 1)*. readline récupère, le lexer découpe (tokens), le parser construit l'arbre, on l'explore (AST), on remplace les `$` (expansion) juste avant le fork, on lie les fd, puis on `execve`.

**217. Pourquoi un AST plutôt qu'une liste plate ?**
Pour hiérarchiser naturellement les priorités (`&&`, `|`, parenthèses) et simplifier l'exécution récursive.

**218. Comment ta grammaire encode-t-elle la priorité et l'associativité ?**
Par la profondeur d'appel : plus la règle est prioritaire, plus elle est appelée profondément. L'associativité gauche est gérée en mettant la récursivité (ou la boucle) à gauche.

**219. Explique `a | b | c` et dessine son AST.**
*(Voir Question 24)*. Le deuxième pipe est la racine, son fils gauche est le premier pipe (qui englobe `a` et `b`), son fils droit est `c`.

**220. Pourquoi `fork()` puis `execve()` ?**
Je fais `fork()` pour dupliquer le shell. Dans la copie (enfant), je fais `execve()` pour charger le nouveau programme. Le parent reste en vie pour gérer la suite.

**221. Explique complètement `ls | wc -l`, notamment les `close()`.**
Parent crée le pipe. Fork 1 (ls) dup fd 1 vers écriture pipe, ferme les fd, execve. Fork 2 (wc) dup fd 0 vers lecture pipe, ferme les fd, execve. Parent ferme les deux bouts du pipe et fait deux waitpid. S'il ne ferme pas, `wc` bloquera.

**222. Pourquoi `cd` doit-il s'exécuter dans le parent ?**
Parce que modifier le répertoire courant dans un enfant est inutile : l'enfant meurt et le shell parent ne bouge pas.

**223. Explique `export X=1 | cat` et pourquoi `X` ne persiste pas.**
Bash forke pour chaque composant d'un pipe. `export` tourne dans un enfant, y modifie l'environnement, puis l'enfant meurt. Le parent (le shell principal) n'a jamais vu la modification.

**224. Explique Ctrl-C vs Ctrl-D : signal, EOF, sigaction, `readline()` et `NULL`.**
Ctrl-C = SIGINT intercepté par `sigaction()`, imprime ^C et relance `readline()`. Ctrl-D = EOF (pas un signal), `readline()` capte une fin de fichier et renvoie `NULL`, entraînant le `exit` du shell.

**225. Pourquoi `SA_RESTART` n'est-il pas utilisé ?**
Parce qu'on veut interrompre `readline()` lors d'un Ctrl-C, pas relancer silencieusement la lecture.

**226. Comment un heredoc interrompu par Ctrl-C communique-t-il son état au parent ?**
L'enfant gérant le heredoc meurt avec le code `130`. Le parent récupère ce `130` avec `waitpid` et arrête l'exécution de l'arbre en cours.

**227. Comment `waitpid()` permet-il de calculer `$?` ?**
Il renseigne `wstatus`. On utilise `WIFEXITED` et `WEXITSTATUS` (code normal) ou `WIFSIGNALED` et `WTERMSIG` (signal) pour fixer la variable globale `$?`.

**228. Explique 128 + signal, 126 et 127.**
- `128 + sig` (ex: 130 pour SIGINT) : terminaison anormale par un signal.
- `127` : commande introuvable.
- `126` : permission refusée (non exécutable).

**229. Qu'est-ce qui survit à `execve()` et pourquoi cela permet-il aux redirections de fonctionner ?**
Les file descriptors (et PID). On `dup2` le stdout vers notre fichier. Quand on `execve`, le fd 1 reste lié au fichier, donc le nouveau programme écrit dedans.

**230. Explique le système de propriété mémoire de ton AST et pourquoi chaque fonction libère exactement ce qu'elle doit libérer.**
Chaque nœud est responsable de libérer ce qu'il contient (tableaux `argv`, redirections). Les opérateurs libèrent leurs enfants. La racine libère l'arbre entier via une fonction récursive `ft_free_node()`.
