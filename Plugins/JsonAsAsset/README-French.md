# JsonAsAsset

[![Discord](https://img.shields.io/badge/Join%20Discord-Collector?color=7289DA&label=JsonAsAsset&logo=discord&logoColor=7289DA&style=for-the-badge)](https://discord.gg/h9s6qpBnUT)
[![GitHub Downloads (all assets, latest release)](https://img.shields.io/github/downloads/JsonAsAsset/JsonAsAsset/latest/total?style=for-the-badge&label=DOWNLOADS&color=blue)](https://github.com/JsonAsAsset/JsonAsAsset/releases)
[![GitHub Repo stars](https://img.shields.io/github/stars/JsonAsAsset/JsonAsAsset?style=for-the-badge&logo=&color=gold)](https://github.com/JsonAsAsset/JsonAsAsset/stargazers)

[![Unreal Engine 5 Supported)](https://img.shields.io/badge/UE5.0+-black?logo=unrealengine&style=for-the-badge&labelColor=grey)](#install)
[![Unreal Engine 4.27 Supported)](https://img.shields.io/badge/4.27-black?logo=unrealengine&style=for-the-badge&labelColor=grey)](#install)

### Description:

Un plugin convivial pour [Unreal Engine](https://www.unrealengine.com/en-US) qui lit les fichiers [JSON](https://www.json.org/json-en.html) extraits de [ FModel](https://fmodel.app), et régénère les actifs tels qu'ils étaient à partir des fichiers du jeu.

✨ [Contributors](#contribute)

### Exemple d'utilisation :

- Portage des **matériaux**, actifs de données, tableaux de données *[(voir plus ici)](#intro)*
- Portage des **effets sonores** sur Unreal Engine
- Automatisation du processus de portage pour les utilisateurs

L'objectif de ce projet est de simplifier le processus pour les communautés de portage et de modding, leur permettant de porter plus facilement des actifs dans Unreal Engine.

-----------------

**Table of Contents**:
<br> 

> 1. [Introduction à JsonAsAsset](#intro)
> 1. [Installation](#installation)  
> 2.1 [Configurer FModel](#setup-fmodel)  
> 2.2 [Paramètres de configuration](#setup-settings)  
> 3. [Récupération locale](#setup-local-fetch)

**Plus** :
<br> 

> - [Erreurs courantes 🐛](#erreurs-communes)

-----------------

<a name="intro"></a>
## 1. Introduction à JsonAsAsset
> [!NOTE]
> Veuillez noter que ce plugin est destiné uniquement à un **usage personnel et éducatif**. Ne l'utilisez pas pour créer ou distribuer des **produits commerciaux** sans obtenir les **licences et autorisations** nécessaires. Il est important de respecter les **droits de propriété intellectuelle** et d'utiliser uniquement les actifs pour lesquels vous êtes **autorisé**. Nous n'assumons aucune responsabilité quant à la manière dont le contenu créé est utilisé.

JsonAsAsset est un plugin Unreal Engine convivial permettant d'importer des actifs à partir de jeux packagés à l'aide de fichiers JSON. Avec une interface élégante, il simplifie le mappage des données JSON aux structures Unreal Engine, prenant en charge les flux de travail manuels et automatisés.

<details>
  <summary>Types d'actifs pris en charge</summary>

###### Sound Classes
- SoundWave

###### Materials
 - Material
 - MaterialFunction
 - MaterialParameterCollection
 - PhysicalMaterial
 - SubsurfaceProfile
     
###### Curve Asset Types
 - CurveFloat
 - CurveTable
 - CurveVector
 - CurveLinearColor
 - CurveLinearColorAtlas

###### Skeleton/Animation Asset Types
 - SkeletalMeshLODSettings
 - Animation (curves, sync markers)
 - Blendspace

###### Data Asset Types
- DataAsset
- DataTable

</details>

<a name="install"></a>
## 2. Installation
> [!TIP]
> JsonAsAsset peut ne pas fonctionner pour toutes les versions d'Unreal Engine 5 ; veuillez vérifier les versions pour la compatibilité. Unreal Engine 4 n'est pas maintenu pour le moment et il n'est pas prévu qu'il soit pris en charge.
> *`(Voir les branches pour les versions actuellement disponibles d'Unreal Engine 4)`*

1. Accédez à la [Page Releases](/../../releases) du plugin.
2. **Téléchargez la version** qui correspond à votre version d'Unreal Engine. S'il **n'existe pas de version correspondant à votre version**, vous devrez [**compiler le plugin vous-même**](https://dev.epicgames.com/community/learning/tutorials/qz93/unreal -plugins de construction de moteurs).
3. Extrayez les fichiers téléchargés dans le dossier Plugins de votre projet. S'il n'y a pas de dossier Plugins, créez-en un dans le répertoire racine de votre projet.
4. Ouvrez votre projet Unreal Engine.
5. Cliquez sur Modifier -> Plugins.
6. Dans la fenêtre Plugins, recherchez « JsonAsAsset » et activez-le.
7. Redémarrez l'éditeur pour que les modifications prennent effet.

<a name="setup-fmodel"></a>
#### 2.1 Configuration du modèle F
Si vous ne l'avez pas déjà fait, installez FModel et configurez-le correctement.
<img align="left" width="150" height="150" src="./Resources/FModelLogo.png?raw=true">
Le format/fichier JSON doit provenir d'un programme qui correspond au format des fichiers d'exportation JSON de FModel :
<br><br>

- [FModel](https://fmodel.app) *(Logiciel pour explorer les jeux Unreal Engine)*

-------------------

Maintenant que vous avez installé FModel et l'avez configuré correctement, nous pouvons continuer à configurer JsonAsAsset pour notre propre projet Unreal Engine.

<a name="setup-settings"></a>
##### 2.2 Paramètres de configuration
<img align="right" width="300" height="180" src=https://github.com/JsonAsAsset/JsonAsAsset/assets/73559984/aad4e86a-6f0b-4e66-aef1-13d30d8215de)>

> [!NOTE]
> Si FModel est installé et configuré correctement, vous pouvez maintenant appuyer sur le bouton Paramètres FModel dans vos paramètres pour le récupérer à partir de là!

Nous devons changer le répertoire d'exportation pour permettre au plugin de faire la différence entre quel est votre répertoire et quel est le chemin de jeu dans lequel il doit le mettre.

Tout d'abord, ouvrez les paramètres du plugin JsonAsAsset (en gros, faites ce qui est sur l'image de droite) et assurez-vous que vous regardez la propriété "Export Directory".

Maintenant, ouvrez FModel et accédez à vos paramètres. `(Paramètres -> Général)` Il y aura un paramètre appelé "Répertoire de sortie", copiez-le et revenez à Unreal Engine. Vous devez maintenant cliquer sur les trois points, accéder au dossier que vous avez copié, accéder au dossier nommé « Exportations », puis appuyer sur « Sélectionner un dossier ».

-------------------

C'est la configuration de base ! Pour importer en masse des actifs et **ce qu'ils référencent**, vous devez configurer [`Local Fetch`](#setup-local-fetch)!

<a name="setup-local-fetch"></a>
## 3. Configuration de la *Récupération locale*
L’exécution de l’API nécessite l’installation d’ASP.NET 8.0 ; veuillez l'installer [ici](https://dotnet.microsoft.com/en-us/download/dotnet/thank-you/runtime-aspnetcore-8.0.1-windows-x64-installer).

<img align="right" width="460" height="156" src=https://github.com/user-attachments/assets/2e3a3680-ccba-4847-9e81-50242085ae63>

> [!TIP]
> Veuillez vous assurer que vous avez le plugin dans le répertoire de votre projet et non dans le moteur.

Avant de pouvoir lancer Local Fetch et commencer à travailler sur des références automatisées, vous devez d'abord saisir toutes les informations sur votre jeu.

Beaucoup de ces paramètres sont similaires à FModel, mais sélectionnez manuellement un fichier ou un répertoire à l'aide du sélecteur de fichiers de l'UE.

<img align="right" width="220" height="156" src=https://github.com/user-attachments/assets/ede73451-9e69-40d9-b1e2-4ee3a00838c9>

###### Launching Local Fetch

> [!IMPORTANT]
> Vous devez lancer Local Fetch via UE et non par le fichier exécutable. La raison en est que le port de l'hôte local est différent lorsque vous le lancez via UE, vous devez donc le faire.

Allez-y et cliquez sur le logo JsonAsAsset (<img width="25" height="25" src=https://github.com/JsonAsAsset/JsonAsAsset/assets/73559984/b90ab71f-d9ac-4349-96eb-620aadf7812f>) et survolez la liste `"Application en ligne de commande"` et appuyez sur `"Exécuter la récupération locale (.EXE)"`.

Une fenêtre devrait apparaître, et une fois que la console indique « [CORE] Running API », Local Fetch a été démarré avec succès !

-----------------------

<a name="common-errors"></a>
#### Common Errors 🐛

<details>
  <summary>Attempted to create a package with name containing double slashes. PackageName: ...</summary>
  
------------
  
Please set your Export Directory to your actual "Output/Exports" directory

***INCORRECT***: T:/FModel/Output

***CORRECT***: T:/FModel/Output/Exports
</details>

<details>
  <summary>Assertion failed: TextureReferenceIndex != INDEX_NONE</summary>

------------
Il s'agit d'un problème connu dans notre code que nous n'avons pas encore entièrement résolu. Bien que les tentatives précédentes pour résoudre ce problème aient échoué, voici une solution partielle pour réduire son apparition :

- Relancez votre projet Unreal Engine, accédez aux paramètres du plugin JsonAsAsset et activez ***"Expose Pins"*** ou ***"Skip Result Node Connection"***. Activez également *** "Autoriser l'enregistrement des packages" ***.
</details>

<details>
  <summary>Local Fetch closes instantly when executing</summary>

------------

Il peut y avoir plusieurs raisons pour lesquelles l'application se ferme automatiquement, mais le problème vient principalement d'un élément manquant :

### 1. Vos paramètres sont erronés et nécessitent des modifications
> Si vous n'avez rien dans les paramètres, comment va-t-il s'exécuter correctement ? Assurez-vous que toutes les informations sur votre jeu sont définies dans les paramètres.

### 2. ASP.NET 8.0 non installé
> L'exécution de l'API nécessite l'installation d'ASP.NET 8.0, veuillez l'installer [ici](https://dotnet.microsoft.com/en-us/download/dotnet/thank-you/runtime-aspnetcore-8.0.2- Windows-x64-installer).

### 3. oo2core manquant
> Exécutez l'API manuellement dans l'explorateur de fichiers une fois, cela devrait télécharger le fichier à utiliser dans l'API. (Plugins/JsonAsAsset/Binaires/Win64/LocalFetch)
</details>

--------------------

<a name="contribute"></a>
#### ✨ Contributors

Thanks go to these wonderful people:

<!-- prettier-ignore-start -->
<!-- markdownlint-disable -->
<table>
  <tbody>
    <tr>
      <td align="center" valign="top" width="14.28%"><a href="https://github.com/Tectors"><img src="https://github.com/Tectors.png" width="100px;" alt="Tector"/><br /><sub><b>Tector</b></sub></a><br/></a></td>
      <td align="center" valign="top" width="14.28%"><a href="https://github.com/GMatrixGames"><img src="https://github.com/GMatrixGames.png" width="100px;" alt="GMatrixGames"/><br /><sub><b>GMatrixGames</b></sub></a><br/></a></td>
      <td align="center" valign="top" width="14.28%"><a href="https://github.com/zyloxmods"><img src="https://github.com/zyloxmods.png" width="100px;" alt="Zylox"/><br /><sub><b>Zylox</b></sub></a><br/></a></td>
      <td align="center" valign="top" width="14.28%"><a href="https://github.com/NathanFelipeRH"><img src="https://github.com/NathanFelipeRH.png" width="100px;" alt="Zylox"/><br /><sub><b>NathanFelipeRH</b></sub></a><br/>Sound Cue Importing</a></td>
      <td align="center" valign="top" width="14.28%"><a href="https://github.com/Tevtongermany"><img src="https://github.com/Tevtongermany.png" width="100px;" alt="Tevtongermany"/><br /><sub><b>Tevtongermany</b></sub></a><br/>JsonAsAsset Logo</a></td>
  </tbody>
</table>

- Thanks to the people who contributed to [UEAssetToolkit](https://github.com/Buckminsterfullerene02/UEAssetToolkit-Fixes)!
