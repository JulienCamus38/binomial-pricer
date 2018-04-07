# binomial-pricer
Développement en parallèle d'un outil de simulation numérique qui utilise la méthode d’arbre pour calculer le prix d’options vanilles.

[Utilisation sur terminal]
Aller dans le dossier src/ puis lancer $make, et enfin exécuter le pricer avec $./pricer

[Utilisation sur IDE]
L'initiative du CMakeLists.txt a été utile dans l'import dans l'IDE QtCreator. Bizarrement, le cmake ne fonctionne pas en ligne de commande directe via le terminal. Ceci est sûrement dû à un problème de lecture du noyau OpenCL depuis le répertoire build/.
