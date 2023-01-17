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
  
  ![image](https://user-images.githubusercontent.com/121939768/212864311-7d76dea6-8f4b-4b8e-91b2-01d91fa9e7be.png)

Le découpage entre la partie logicielle et la partie matérielle se fait selon le schéma suivant :

![image](https://user-images.githubusercontent.com/121939768/212871716-b8f74743-0524-45e7-9d4a-2a54350af11a.png)

Chaque bloc (ou IP) possède une fonctionnalité spécifique sur notre cible. La Clock va venir définir la cadence de communication entre chaque bloc et au sein d'un même bloc. Le Nios2 est le softcore que l'on va programmer sur le FPGA et qui va venir éxecuter notre script en C. Le Onchip Memory va venir sauvegarder notre soft avec toutes ses variables et bibliothèques. Enfin les PIO sont créés pour raccorder les périphériques avec le Nios. La particularité ici est l'utilisation de l'IP qui va permettre de venir communiquer en I2C.

### 1-- Test de communication en I2C

La première étape était donc de voir si la connection I2C était paramétrée pour voir les informations venant de l'accéléromètre. Pour cela, je suis venu lire le registre m'indiquant l'adresse de l'accéléromètre.
