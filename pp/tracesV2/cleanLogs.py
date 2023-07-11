import os

def parcourir_repertoire(repertoire):
    for dossier, sous_dossiers, fichiers in os.walk(repertoire):
        for fichier in fichiers:
            chemin_fichier = os.path.join(dossier, fichier)
            if "dessin" not in chemin_fichier:
                print("process file: "+chemin_fichier)
                cleanF = ""
                with open(chemin_fichier, 'r') as f:
                    for line in f:
                        if not any([x in line for x in ["start", "programming_language_used", "end", "paused", "delayed", "won"]]):
                            cleanF = cleanF + line.strip('\t')
                with open(chemin_fichier, 'w') as f:
                    f.writelines(cleanF)

# Exemple d'utilisation
repertoire = "example"  # Remplacez par le chemin de votre répertoire
parcourir_repertoire(repertoire)
