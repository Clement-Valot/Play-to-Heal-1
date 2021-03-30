# Play to Heal

## GitHub


# Utilisation normale avec Git BASH


```
git init
```
Pour initialiser le répertoire et permettre d'utiliser git avec

```
git remote add origin https://github.com/RemiEC/<nom projet>.git
```
Va créer un "flux" entre votre dossier local et le git du projet
```
git remote -v
```
Devrait afficher tous les flux reliés au dossier local dans lequel vous êtes, ici ça devrait donc afficher le flux upstream
```
git fetch origin
```
Va vous ajouter les références du git projet (notamment les infos des branches) et vous permettre de les utiliser


POUR RECUPERER DES FICHIERS DEPUIS GIT
```
git pull origin
```
Récupérer ce qui est en remote et le mettre en local

POUR EN ENVOYER

```
git add -A
```
Permet de stage tous les changements faits

```
git commit -m "<message>"
```
Permet de mettre ces changements staged dans un commit
  
```
git push origin master
```

Permet d'envoyer le commit sur Github

### Initialisation avec VSCode


Pour que tout fonctionne vous devez créer un répertoire local ni sur OneDrive ni avec un chemin contenant des caractères spéciaux.
Vous devrez ensuite installer Visual Studio Code (https://code.visualstudio.com/) + Git (https://git-scm.com/download/win).
Quand vous lancez Visual Studio Code il faut que vous choisissiez le type de terminal. 
Tapez CTRL-SHIFT-P et une box devrait s'ouvrir en haut de votre éditeur. Tapez "Select Default Shell" et choisissez la shell bash
Vous pourrez comme ça avoir le terminal git bash directement intégré dans VSC plutôt que devoir gérer deux fenêtres distinctes.

////////////////////////////////
SI BASH N'APPARAIT PAS DANS LES SHELLS PROPOSEES

Ce problème survient sur plusieurs types de machine (notamment la mienne).
Il faut alors aller dans les Settings (en passant par le rouage en bas à gauche) et scroll jusqu'à ce que vous trouviez un lien 'Edit in settings.json'. Cliquez dessus, cela vous ouvrira directement le fichier settings.json sans que vous ayez à le chercher dans votre machine
Vous devriez avoir dans une des lignes du répertoire quelque chose de similaire à :
"terminal.integrated.shell.windows": "C:\\Program Files (x86)\\Git\\bin\\bash.exe",

Il s'agit du chemin qui permet au terminal de savoir quel shell utiliser, vous devriez avoir un chemin différent qui ne mène pas à bash.
Il faut donc que vous remplaciez le chemin d'exemple que j'ai indiqué par votre chemin vers bash.exe qui aura été installé en même temps que Git. Votre chemin devrait être similaire au mien donc je vous le laisse à titre d'exemple (attention à mettre des \\ ou des / dans le chemin sinon il ne sera pas reconnu)
////////////////////////////////

# Utilisation normale dans VS Code

Pour utiliser GitHub avec Visual Studio Code il vous suffit de quelques notions simples.


Quand vous modifiez un fichier track par GitHub, un petit 1 va apparaître sur la gauche de votre écran (c'est le source control)

Les modifications sont envoyés à Git par paquets appelés 'commits', en général on veut envoyer plusieurs modifications d'un coup (des lignes en plus sur un fichier, d'autres en moins sur un autre, etc)

Pour ce faire allez dans l'onglet Source control et cliquez sur le '+' à côté des changements que vous voulez ajouter ('stage') au prochain commit (vous pouvez toujours revenir en arrière et en retirer un du commit).

Vous devez ensuite indiquer un commit message (qui doit être un résumé bref des modifications qu'apporte ce commit) puis cliquer sur la checkmark au dessus pour confirmer le commit

Si vous regardez en bas à gauche de votre écran vous devriez voir une sorte d'ouroboros, il indique le nombre de commit de retard que votre version du projet a ainsi que le nombre de commit d'avance que vous avez.

En cliquant dessus, vous faites à la fois un 'pull' (récupérer le contenu des commits de retard) et un 'push' (envoyer vos propres commits).


Vous avez maintenant des bases de GitHub suffisantes pour travailler en autonomie !
  
  # ATTENTION
  
  - Toujours vérifier la branche sur laquelle on est
  - Pull AVANT de faire le moindre changement, sinon vous risquez d'avoir des merges conflicts. Ce n'est généralement pas difficile à résoudre mais cela peut prendre un peu de    temps
  - Pas de panique si vous ratez quelque chose et supprimez des fichiers sur le Git, il est doté d'un système de version control qui permet de restaurer des versions antérieures du repo. A moins de le faire exprès vous ne pourrez pas tout détruire.
  - Les messages des commits doivent être clair (quoi, pourquoi, comment) pour éviter qu'on soit obligé de passer dernière pour que chacun puisse comprendre ce qui a changé 
  - Nous n'utiliserons pas plusieurs branches car nous codons tous les mêmes fonctionnalités ensemble. Nous éviterons ainsi le process de merge request/approval mais sachez que      cela existe
  
# Extensions recommandées

  - Better Comments
  - Bracket pair colorizer
  - Git History
  - Git Lens
  - Material icon theme
  - Output colorizer
