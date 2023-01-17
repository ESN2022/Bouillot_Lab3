# Bouillot_Lab3 - Advanced co-design project

Sommaire :

a. Introduction

b. System architecture with QSYS components and their interaction, HW blocks, design choices

c. Conclusion, progress and result


## Introduction

Dans ce troisième lab je vais venir me perfectionner un peu plus en co-design. Je vais venir utiliser un nouveau périphérique : l'accéléromètre ADXL345. Je vais ensuite venir afficher les différentes accélérations des différents axes sur les afficheurs 7 segments. Les difficultés vont alors venir de la gestion des conversions mais surtout de la lecture sur le bus I2C. La cible de ce lab est toujours une carte DE10-Lite configurée à l'aide du logiciel Quartus 18.1 et de son environnement.


## System Architecture
La configuration est la même que celle du lab 2 (cf répertoire Bouillot_lab1).
Ce qui vient changer est la configuration sous Platforme Designer. Je vient donc choisir :
  - Nios 2
  - Clock (permettant de cadencer tous les sytèmes)
  - Onchip memory
  - Jtag (permettant la communication PC - Carte)
  - Un ou lusieurs PIO pour la gestion de l'affichage sur les afficheurs 7 segments
  - Un timer
  - une IP opencore_I2C permettant la communication via I2C
 L'architecture complète du système est final est :
  
  ![image](https://user-images.githubusercontent.com/121939768/212864311-7d76dea6-8f4b-4b8e-91b2-01d91fa9e7be.png)

Le découpage entre la partie logicielle et la partie matérielle se fait selon le schéma suivant :

![image](https://user-images.githubusercontent.com/121939768/212871716-b8f74743-0524-45e7-9d4a-2a54350af11a.png)

Chaque bloc (ou IP) possède une fonctionnalité spécifique sur notre cible. La Clock va venir définir la cadence de communication entre chaque bloc et au sein d'un même bloc. Le Nios2 est le softcore que l'on va programmer sur le FPGA et qui va venir éxecuter notre script en C. Le Onchip Memory va venir sauvegarder notre soft avec toutes ses variables et bibliothèques. Enfin les PIO sont créés pour raccorder les périphériques avec le Nios. La particularité ici est l'utilisation de l'IP qui va permettre de venir communiquer en I2C.

### 1-- Test de communication en I2C

La première étape était donc de voir si la connection I2C était paramétrée pour voir les informations venant de l'accéléromètre. Pour cela, je suis venu lire le registre m'indiquant l'adresse de l'accéléromètre. On vérifie alors que l'acknowledge soit bien à  pour pouvoir avoir accès à l'adresse de l'ADXL345.

![image](https://user-images.githubusercontent.com/121939768/212903188-9769d1e4-7540-438e-970e-5d99ecc97cc3.png)

### 2- Lecture des axes, conversion en variable signée et affichage sur les afficheurs 7 segments

Je viens alors implanter différentes fonctions (en langage C) comme présenté dans le découpage de la partie logicielle. Les pièges à faire attention viennent principalement de la gestion des valeurs non signée, signée et converties en mili g. Pour cela il faut donc être très rigoureux, d'où l'intérêt de bien découper les différentes fonctionnalités dans plusieurs fonctions.

Aussi,communication est gérée par l’IP opencoresi2c. Les fonctions essentielles sont donc décrite dans le fichier .h de l'IP.
L’initialisation de la communication : void I2C_init(alt_u32 base,alt_u32 clk,alt_u32 speed),
-	L’initialisation de la communication : void I2C_init(alt_u32 base,alt_u32 clk,alt_u32 speed),
-	Le bit de start ainsi que le registre et le write bit: int I2C_start(alt_u32 base, alt_u32 add, alt_u32 read),
-	La lecture : alt_u32 I2C_read(alt_u32 base,alt_u32 last),
-	L’écriture : alt_u32 I2C_write(alt_u32 base,alt_u8 data, alt_u32 last).

Cette partie s'appuie principalement sur la fonction nommée read_axis qui permet la lecture des deux registres d'un des 3 axes. Cette fonction fait appel à une seconde fonction dédiée à la lecture sur le bus I2C (read_data) sur un seul octet. Enfin la dernière fonction nommée axis_calc permet de réaliser toutes les opérations de conversions, à savoir :
 - Concaténation des valeurs provenant des deux registres LSB et MSB d'un axe
 - Complément à 2 permettant de convertir une valeur signée à partir d'une valeur non signée

Enfin la valeur en mili g est affichée sur les afficheurs 7 segments, toutes les valeurs des axes sont envoyé via UART.

### 3 Conversion en mili g et écriture dans les registres d'offset

Lors de cette partie j'ai ajouté la conversion des axes en mili g via la donnée issue de la datasheet, à savoir 4.0 mg/LSB. Je vient maintenant calibrer l'accéléromètre. Pur cela, je lance un terminal nios et j'observe les valeurs envoyées en UART. Comme la carte est posée à plat sur la table, celle-ci n'est soumise qu'à la gravité sur l'axe Z (soit environ 1000 mg sur l'axe Z). Cette situation nous permet donc de calibrer à la fois l'axe X et l'axe Y car ces axes doivent être à 0 g.
On vient donc faire la moyenne des valeurs envoyées par l'accéléromètre, puis on divise par 15.6 pour obtenir l'offset nécessaire. La division par 15.6 est nécessaire car, selon la datasheet, chaque nombre entré dans les registres d'offset sera multiplié par 15.6. 

Par exemple, sur l'axe X, on voit que l'erreur moyenne est de -32, soit un drift constant de -32/15.6= -2. Il faudra alors écrire 2 dans le registre d'offset de l'axe X.
Une fois fait pour l'axe X, j'ai fais la même opération pour les axes Y et Z.

### 4- Ajout des interruption

Dernière étape est l'ajout des interruptions générée par le timer 0 et le bouton poussoir (PIO 1). Les maccros usleep sont remplacées par les interruptions du timer et le bouton poussoir permet de changer l'axe affiché sur les afficheurs 7 segments.

## Conclusion, progress and result

Ma progression pour ce troisième lab est la suivante :

  - Gestion de la communication en I2C
  - Lecture des axes et affichage sur les afficheurs 7 segments
  - Conversion en mili g et gestion des offsets
  - Ajout d'interruptions (timer et bouton poussoir)

