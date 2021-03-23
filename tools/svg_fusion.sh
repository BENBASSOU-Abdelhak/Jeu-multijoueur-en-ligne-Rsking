#!/bin/sh

# script de fusion d'images svg, avec ajout des attributs id et class
# chaque image svg d'entrée ne doit contenir qu'une forme i.e. une balise '<path'
# input :
#   $1: dossier avec images svg d'entrées 
#   $2: nom du fichier de sortie de fusion des images (sans l'extension '.svg')
# output : image fusion.svg contenant toutes les formes de $1 avec pour chaque
#   forme un attribut id et class rempli


if [ "$#" -ne 2 ]
then
    echo "usage: $0 <dossier input> <fichier repr. logique>"
return
elif [ ! -d $1 ]
then
    echo "Dossier n'existe pas"
return
fi

NBREG=$(sed -n 3p $2 | bc)

echo '<?xml version="1.0" standalone="no"?>
<!DOCTYPE svg PUBLIC "-//W3C//DTD SVG 20010904//EN"
"http://www.w3.org/TR/2001/REC-SVG-20010904/DTD/svg10.dtd">
<svg version="1.0" xmlns="http://www.w3.org/2000/svg"
viewBox="0 0 1920.000000 1080.000000"
preserveAspectRatio="xMidYMid meet">
<g transform="translate(0.000000,1080.000000) scale(0.100000,-0.100000)"
fill="#000000" stroke="none">' > fusion.svg

for fichier in `cd $1; ls -1v *.svg`
do
    if [ $(sed -n '/<path/=' $1/$fichier | wc -w) -gt 1 ]
    then
        echo "le fichier $1/$fichier contient plus d'une balise path !"
        return
    fi
    DEBUT=$(($(sed -n '/<path/=' $1/$fichier) - 1))
    FIN=$(($(sed -n '/<\/g>/=' $1/$fichier) - DEBUT))
    ID=$(echo $fichier | cut -f1 -d '.' | bc)

    LIGNE=$(cat $2 | sed "1,$(($NBREG + 3)) d" | sed -n "1,$NBREG p" | grep -wn "$ID" | cut -d':' -f1 | bc)
    REG=$(sed -n "$((3 + $NBREG + $NBREG + $LIGNE)) p" $2)

    echo "\n" >> fusion.svg
    cat $1/$fichier | sed "1,${DEBUT}d" | sed "${FIN},$"d | sed "1 s/<path/<path id=\"$ID\" class=\"$REG\" /" >> fusion.svg
done

echo "</g>\n</svg>" >> fusion.svg