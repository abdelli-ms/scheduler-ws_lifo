### Commandes d'execution:
Après avoir compiler le programme avec la commande make, on peut choisir d'executer:
* _{Ordonnanceur LIFO}_ avec `./lifo/lifo [-n size] [-t threads] [-s]`
* _{Ordonanceur par work stealing}_ avec `./ws/ws [-n size] [-t threads] [-s]`
exemple:
    * pour executer lifo en précisant n=20000 avec 4 thread: `./lifo/lifo -n 20000 -t 4`
    * pour executer lifo en précisant n= 20000 avec le quicksort serial: `./lifo/lifo -n 20000 -s`
### Suppression des fichiers objets et des executables:
`make clean`
### Répertoire du projet: 
Les fichiers correspondant à l'ordonanceur par Work stealing se trouve dans le sous répertoire ws et les fichier correspondant à  l'ordonnancer LIFO se trouve dans le sous répertoire lifo.
### Composition
En binôme:

