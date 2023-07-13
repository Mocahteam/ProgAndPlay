import subprocess
import os 
import matplotlib.pyplot as plt
import numpy as np
from mpl_toolkits.mplot3d import Axes3D
import seaborn as sns
import shutil
import sys
from decimal import Decimal, getcontext


global g_exploredMap, g_episilon, g_tab_parametersToBestResultPos, g_tab_parametersToColorId, g_map_bestSolutionToColorId, g_colorId

g_episilon = 0.1

g_exploredMap = {}
g_tab_parametersToBestResultPos = np.zeros((int(1+(1/g_episilon)), int(1+(1/g_episilon))))
g_tab_parametersToColorId = np.zeros((int(1+(1/g_episilon)), int(1+(1/g_episilon))))
g_map_bestSolutionToColorId = {}
g_colorId = 1

# la classe Point
# un objet Point représente par sa valeur de war et ws
class Point:
	def __init__(self, war, ws, pb):
		self.war = war
		self.ws = ws
		self.pb = pb

	def __hash__(self):
		return hash(str(self.war) + str(self.ws) + str(self.pb))

	def __eq__(self, other):
		return abs(self.war-other.war) < 0.001 and abs(self.ws-other.ws) < 0.001 and abs(self.pb-other.pb) < 0.001

# la classe Cube
# un objet Cube représente un zone rectangle avec deux pointes en diagonale
class Cube:
	def __init__(self, war_from, war_to, ws_from, ws_to, pb_from, pb_to):
		self.war_from = Decimal(war_from).quantize(Decimal('1.00'))
		self.war_to = Decimal(war_to).quantize(Decimal('1.00'))
		self.ws_from = Decimal(ws_from).quantize(Decimal('1.00'))
		self.ws_to = Decimal(ws_to).quantize(Decimal('1.00'))
		self.pb_from = Decimal(pb_from).quantize(Decimal('1.00'))
		self.pb_to = Decimal(pb_to).quantize(Decimal('1.00'))

	def __hash__(self):
		return hash(str(self.war_from) + str(self.war_to) + str(self.ws_from) + str(self.ws_to) + str(self.pb_from) + str(self.pb_to))

	def __eq__(self, other):
		return abs(self.war_from-other.war_from) < 0.001 and abs(self.war_to-other.war_to) < 0.001 and abs(self.ws_from-other.ws_from) < 0.001 and abs(self.ws_to-other.ws_to) < 0.001 and abs(self.pb_from-other.pb_from) < 0.001 and abs(self.pb_to-other.pb_to) < 0.001


# \brief Lit le fichier et retourne son contenu sous la forme d'une liste. Chaque élément de la liste contient une ligne du fichier
#
# @file : nom d'un fichier contenat une trace d'exécution
def tokenizeFile(file):
	ret = []
	with open(file) as f:
		for line in f:
			ret.append(line[0:-1]) # exclure le \n à la fin de la ligne
	return ret

# \brief explore le fichier "resultFile" et recherche une compression à la solution "l_solution". Si elle existe retourne le numéro de la compression ou 20000 si elle n'est pas trouvée ou 100000 si le log contient un "Over Time"
#
# @resultFile : nom du fichier contenant des résultats de compression
# @l_solution : représente la solution de référence sous la forme d'une liste
def getBestResultPosEqualToSolution(resultFile, l_solution):
	num=0
	last_score = "***"
	overTime = False
	with open(resultFile) as f:
		for line in f:
			if "Over Time" in line:
				# On arrive sur un "Over Time" qui signifie que le processus de compression a avorté par manque de temps. Le fichier log est donc incomplet
				overTime = True
			if (line[0]=="["):
				# Extraction de ce résultat sous la forme d'une liste de tokens
				tokens = line.split(" ")
				best = []
				for t in tokens[2:-1]:
					if ("Sequence(" in t):
						best.append("Sequence()")
					else:
						best.append(t)
				# Vérifier si l'id de ce résultat est différent du précédent. Si oui enregistrer son id
				if(tokens[1]!=last_score):
					num += 1
					last_score = tokens[1]
				# Si ce résultat est égale à la solution, on retourne son id et on stoppe la recherche
				if (best == l_solution):
					return num
		if overTime:
			return 100000
		else:
			# Aucun résultat identique à la solution n'a été trouvé
			return 20000

# \brief Lire la solution et retoune la meilleure solution
#
# @resultFile : nom du fichier contenant des résultats de compression
def get_bestSolution(resultFile):
	with open(resultFile) as f:
		for line in f:
			# dans le cas où on dépasse le temps maximale autorisé nous retounons None
			if "Over Time" in line:
				return None
			if (line[0]=="["):
				return ' '.join(line.split(' ')[6:-1]) # extraction du résultat initialement contenu entre Root et EndRoot
	return None

# \brief Lire la solution et retouner les meilleures solutions sous la forme d'un liste
#
# @resultFile : nom du fichier contenant des résultats de compression
def get_allSolutions(resultFile):
	ret = []
	with open(resultFile) as f:
		for line in f:
			# dans le cas où on dépasse le temps maximale autorisé nous retounons None
			if "Over Time" in line:
				break
			if (line[0]=="["):
				ret.append(' '.join(line.split(' ')[6:-1])) # extraction du résultat initialement contenu entre Root et EndRoot
	return ret

# \brief arroundi un nombre à un nombre de chiffre après la virgule fixe
def round_to_multiple(number):
    global g_episilon
    return Decimal(g_episilon * round(number / Decimal(g_episilon)))

# \brief Essayer d'obtenir la solution avec un objet Point, si nous avons déjà eu la solution de ce point nous retournons directement la solution, sinon nous allons exécuter myTestParser avec les paramètres du point et enregistrer la solution dans un map
#
# @point : le point sous la forme d'une combinaison war/ws à tester
# @targetFileName : nom du fichier log à analyser
# @l_solution : représente la solution de référence sous la forme d'une liste. Si égale à None le résultat de la compression n'est pas comparé à la solution.
def get_from_map(point, targetFileName, l_solution=None):
	global g_exploredMap, g_map_bestSolutionToColorId, g_colorId, g_episilon, g_tab_parametersToBestResultPos, g_tab_parametersToColorId
	# dans le cas les paramètres ne sont plus légitimes
	if(point.war>1 or point.ws>1 or point.pb>1 or point.war<0 or point.ws<0 or point.pb<0):
		return None
	# si nous avons déjà eu la solution nous la retounons directement
	if point in g_exploredMap.keys():
		return g_exploredMap[point]
	# sinon nous faisons l'essaie avec les paramètres du point
	else:
		i = round(float(point.war)/g_episilon)
		j = round(float(point.ws)/g_episilon)
		k = round(float(point.pb)/g_episilon)
		war = point.war
		ws = point.ws
		pb = point.pb
		print("Call parser with parameters\twar: "+str(war)+"   \tws: "+str(ws)+"   \tpb: "+str(pb))
		file_exe = "myParserTest.exe "+ targetFileName+".log -sc 0.2 -cl 10 -tl 10 -gr " +str(war) + " -ws "+str(ws) + " -pb "+str(pb)+" -disableLogs"
		child = subprocess.Popen(file_exe,shell=True)
		child.wait()
		# copy du résultat
		shutil.copyfile("./debugParser.log", "./result/rectangle/"+targetFileName+"/debugParser_"+targetFileName+"_war"+str(war) + "_ws"+str(ws) + "_pb"+str(pb)+".log")
		
		# si nous voulons vérifier la correction de solution
		if(l_solution != None):
			g_tab_parametersToBestResultPos[i][j][k]=getBestResultPosEqualToSolution("./debugParser.log",l_solution)

		allSolutions = get_allSolutions("./debugParser.log")
		bestSolution = get_bestSolution("./debugParser.log")

		# si nous avons déjà eu cette solution compresée, nous mettons la même couleur pour signifier que ces deux solutions sont identiques
		if(bestSolution in g_map_bestSolutionToColorId.keys()):
			g_tab_parametersToColorId[i][j][k] = g_map_bestSolutionToColorId[bestSolution]
		# sinon nous mettons la valeur de nouveau g_colorId qui représente l'identique de solution courrant et nous augmentons la valeur g_colorId
		else:
			g_map_bestSolutionToColorId[bestSolution] = g_colorId
			g_tab_parametersToColorId[i][j][k] = g_colorId
			g_colorId += 1
		g_exploredMap[point] = allSolutions
		return allSolutions

# \brief Compresse les logs contenus dans le fichier "targetFileName" en explorant les paramètres war et ws de manière dichotomique.
#
# @targetFileName : Nom du fichier log à compresser
# @l_solution : représente la solution de référence sous la forme d'une liste. Si égale à None le résultat de la compression n'est pas comparé à la solution.
def search_war_ws_by_rect(targetFileName, l_solution=None):
	global  g_exploredMap, g_episilon, g_tab_parametersToBestResultPos, g_tab_parametersToColorId, g_map_bestSolutionToColorId, g_colorId
	g_exploredMap = {}
	g_tab_parametersToBestResultPos = np.zeros((int(1+(1/g_episilon)), int(1+(1/g_episilon)), int(1+(1/g_episilon))))
	g_tab_parametersToColorId = np.zeros((int(1+(1/g_episilon)), int(1+(1/g_episilon)), int(1+(1/g_episilon))))
	g_map_bestSolutionToColorId = {}
	g_colorId = 1

	if(not os.path.exists("./result/rectangle/"+targetFileName)):
		os.makedirs("./result/rectangle/"+targetFileName)
	else:
		for f in os.listdir("./result/rectangle/"+targetFileName):
			os.remove(os.path.join("./result/rectangle/"+targetFileName, f))

	queue = []
	queue.append(Cube(0, 0.5, 0, 0.5, 0, 0.5))
	queue.append(Cube(0, 0.5, 0, 0.5, 0.5, 1))
	queue.append(Cube(0, 0.5, 0.5, 1, 0, 0.5))
	queue.append(Cube(0, 0.5, 0.5, 1, 0.5, 1))
	queue.append(Cube(0.5, 1, 0, 0.5, 0, 0.5))
	queue.append(Cube(0.5, 1, 0, 0.5, 0.5, 1))
	queue.append(Cube(0.5, 1, 0.5, 1, 0, 0.5))
	queue.append(Cube(0.5, 1, 0.5, 1, 0.5, 1))
	decimal_episilon = round_to_multiple(Decimal(g_episilon))
	while(len(queue)>0):
		r = queue.pop(0)
		# Récupération des 8 points à analyser en coordonnées war, ws et pb
		#     p7--------p8
		#    /|         /|
		#   / |        / |
		#  /  |       /  |  ws 
		# p3--------p4   |   x
		# |   |      |   |   |  x pb
		# |   p5-----|--p6   | /
		# |  /       |  /    |/
		# | /        | /     +------x war
		# |/         |/
		# p1--------p2
		p1 = Point(r.war_from, r.ws_from, r.pb_from)
		p2 = Point(r.war_to, r.ws_from, r.pb_from)
		p3 = Point(r.war_from, r.ws_to, r.pb_from)
		p4 = Point(r.war_to, r.ws_to, r.pb_from)
		p5 = Point(r.war_from, r.ws_from, r.pb_to)
		p6 = Point(r.war_to, r.ws_from, r.pb_to)
		p7 = Point(r.war_from, r.ws_to, r.pb_to)
		p8 = Point(r.war_to, r.ws_to, r.pb_to)
		# calcul des 8 compressions (ou récupération dans la map si déjà calculé)
		sol1 = get_from_map(p1,targetFileName,l_solution)
		sol2 = get_from_map(p2,targetFileName,l_solution)
		sol3 = get_from_map(p3,targetFileName,l_solution)
		sol4 = get_from_map(p4,targetFileName,l_solution)
		sol5 = get_from_map(p5,targetFileName,l_solution)
		sol6 = get_from_map(p6,targetFileName,l_solution)
		sol7 = get_from_map(p7,targetFileName,l_solution)
		sol8 = get_from_map(p8,targetFileName,l_solution)
		# calcul des positions intermédiaires
		war_halfGap = round_to_multiple((p2.war-p1.war)/2)
		ws_halfGap = round_to_multiple((p3.ws-p1.ws)/2)
		pb_halfGap = round_to_multiple((p5.pb-p1.pb)/2)
		# si toutes les solution sont égales, il suffit de passer au cube suivant dans la queue
		if (sol1 == sol2 and sol1 == sol3 and sol1 == sol4 and sol1 == sol5 and sol1 == sol6 and sol1 == sol7 and sol1 == sol8):
			continue
		# si la face devant est homogène mais différente d'un point en arrière ET qu'on peut encore découper sur l'axe de la profondeur
		if (sol1 == sol2 and sol1 == sol3 and sol1 == sol4 and (sol1 != sol5 or sol3 != sol7 or sol2 != sol6 or sol4 != sol8) and pb_halfGap >= decimal_episilon*2):
			# Construction d'un cube pour l'avant
			#     p7--------p8
			#    /|         /|
			#   +----------+ |
			#  /          /| |
			# p3--------p4 | |
			# |          | | |
			# |          | |p6
			# |          | |/
			# |          | +
			# |          |/
			# p1--------p2
			queue.append(Cube(p1.war, p2.war, p1.ws, p3.ws, round_to_multiple(p1.pb + decimal_episilon), round_to_multiple(p1.pb + pb_halfGap)))
		# si la face arrière est homogène mais différente d'un point en avant ET qu'on peut encore découper sur l'axe de la profondeur
		if (sol5 == sol6 and sol5 == sol7 and sol5 == sol8 and (sol1 != sol5 or sol3 != sol7 or sol2 != sol6 or sol4 != sol8) and pb_halfGap >= decimal_episilon*2):
			# Construction d'un cube pour l'arrière
			#     p7--------p8
			#    /          /|
			#   +----------+ |
			#  /|         /| |
			# p3--------p4 | |
			# | |        | | |
			# | |        | |p6
			# | |        | |/
			# | +--------|-+
			# |/         |/
			# p1--------p2
			queue.append(Cube(p5.war, p6.war, p5.ws, p7.ws, round_to_multiple(p3.pb + pb_halfGap + decimal_episilon), round_to_multiple(p7.pb - decimal_episilon)))
		# si la face de dessous est homogène mais différente d'un point en dessus ET qu'on peut encore découper sur l'axe vertical
		if (sol1 == sol2 and sol1 == sol5 and sol1 == sol6 and (sol1 != sol3 or sol2 != sol4 or sol5 != sol7 or sol6 != sol8) and ws_halfGap >= decimal_episilon*2):
			# Construction d'un cube pour le dessous
			#     p7--------p8
			#    /|         /|
			#   / |        / |
			#  /  +-------/--+
			# p3--------p4  /|
			# | /        | / |
			# |/         |/ p6
			# +----------+  /
			# |          | /
			# |          |/
			# p1--------p2
			queue.append(Cube(p1.war, p2.war, round_to_multiple(p1.ws + decimal_episilon), round_to_multiple(p1.ws + ws_halfGap), p1.pb, p5.pb))
		# si la face de dessus est homogène mais différente d'un point en dessous ET qu'on peut encore découper sur l'axe vertical
		if (sol3 == sol4 and sol3 == sol7 and sol3 == sol8 and (sol1 != sol3 or sol2 != sol4 or sol5 != sol7 or sol6 != sol8) and ws_halfGap >= decimal_episilon*2):
			# Construction d'un cube pour le dessus
			#     p7--------p8
			#    /          /|
			#   /          / |
			#  /          /  +
			# p3--------p4  /|
			# |          | / |
			# |          |/-p6
			# +----------+  /
			# | /        | /
			# |/         |/
			# p1--------p2
			queue.append(Cube(p3.war, p4.war, round_to_multiple(p1.ws + ws_halfGap + decimal_episilon), round_to_multiple(p3.ws - decimal_episilon), p3.pb, p7.pb))
		# si la face de gauche est homogène mais différente d'un point à droite ET qu'on peut encore découper sur l'axe horizontal
		if (sol1 == sol3 and sol1 == sol5 and sol1 == sol7 and (sol1 != sol2 or sol3 != sol4 or sol5 != sol6 or sol7 != sol8) and war_halfGap >= decimal_episilon*2):
			# Construction d'un cube pour la gauche
			#     p7---+----p8
			#    /    /|    /|
			#   /    / |   / |
			#  /    /  |  /  |
			# p3---+----p4   |
			# |    |   | |   |
			# |    |   +-|--p6
			# |    |  /  |  / 
			# |    | /   | /  
			# |    |/    |/
			# p1---+----p2
			queue.append(Cube(round_to_multiple(p1.war + decimal_episilon), round_to_multiple(p1.war + war_halfGap), p1.ws, p3.ws, p1.pb, p5.pb))
		# si la face de droite est homogène mais différente d'un point à gauche ET qu'on peut encore découper sur l'axe horizontal
		if (sol2 == sol4 and sol2 == sol6 and sol2 == sol8 and (sol1 != sol2 or sol3 != sol4 or sol5 != sol6 or sol7 != sol8) and war_halfGap >= decimal_episilon*2):
			# Construction d'un cube pour la droite
			#     p7----+---p8
			#    /|    /    /|
			#   / |   /    / |
			#  /  |  /    /  |
			# p3----+---p4   |
			# |   | |    |   |
			# |   p5|    |  p6
			# |  /  |    |  / 
			# | /   |    | /  
			# |/    |    |/
			# p1----+---p2
			queue.append(Cube(round_to_multiple(p1.war + war_halfGap + decimal_episilon), round_to_multiple(p2.war - decimal_episilon), p2.ws, p4.ws, p2.pb, p6.pb))
		# si l'arête avant/bas est homogène mais différente d'un point en dessus et en arrière ET qu'on peut encore découper sur l'axe de la profondeur ou vertical
		if (sol1 == sol2 and (sol1 != sol3 or sol2 != sol4) and (sol1 != sol5 or sol2 != sol6) and (ws_halfGap >= decimal_episilon or pb_halfGap >= decimal_episilon)):
			# Construction d'un cube en bas devant
			#     p7--------p8
			#    /|         /|
			#   / |        / |
			#  /  |       /  |
			# p3--------p4   |
			# | +--------|-+ |
			# |/         |/|p6
			# +----------+ |/ 
			# |          | +  
			# |          |/
			# p1--------p2
			queue.append(Cube(p1.war, p2.war, p1.ws, round_to_multiple(p1.ws + ws_halfGap), p1.pb, round_to_multiple(p1.pb + pb_halfGap)))
		# si l'arête arrière/bas est homogène mais différente d'un point en dessus et en avant ET qu'on peut encore découper sur l'axe de la profondeur et vertical
		if (sol5 == sol6 and (sol5 != sol7 or sol6 != sol8) and (sol5 != sol1 or sol6 != sol2) and (ws_halfGap >= decimal_episilon or pb_halfGap >= decimal_episilon)):
			# Construction d'un cube en bas devant
			#     p7--------p8
			#    /|         /|
			#   / |        / |
			#  /  +-------/--+
			# p3--------p4  /|
			# | +--------|-+ |
			# | |        | |p6
			# | |        | |/ 
			# | +--------|-+  
			# |/         |/
			# p1--------p2
			queue.append(Cube(p5.war, p6.war, p5.ws, round_to_multiple(p5.ws + ws_halfGap), round_to_multiple(p1.pb + pb_halfGap + decimal_episilon), p5.pb))
		# si l'arête arrière/haut est homogène mais différente d'un point en dessous et en avant ET qu'on peut encore découper sur l'axe de la profondeur et vertical
		if (sol7 == sol8 and (sol7 != sol5 or sol8 != sol6) and (sol7 != sol3 or sol8 != sol4) and (ws_halfGap >= decimal_episilon or pb_halfGap >= decimal_episilon)):
			# Construction d'un cube en haut arrière
			#     p7--------p8
			#    /          /|
			#   +----------+ |
			#  /|         /| +
			# p3--------p4 |/|
			# | +--------|-+ |
			# |   p5-----|--p6
			# |  /       |  / 
			# | /        | /  
			# |/         |/
			# p1--------p2
			queue.append(Cube(p7.war, p8.war, round_to_multiple(p5.ws + ws_halfGap + decimal_episilon), p7.ws, round_to_multiple(p3.pb + pb_halfGap + decimal_episilon), p7.pb))
		# si l'arête avant/haut est homogène mais différente d'un point en dessous et en arrière ET qu'on peut encore découper sur l'axe de la profondeur et vertical
		if (sol3 == sol4 and (sol1 != sol3 or sol2 != sol4) and (sol3 != sol7 or sol4 != sol8) and (ws_halfGap >= decimal_episilon or pb_halfGap >= decimal_episilon)):
			# Construction d'un cube en haut devant
			#     p7--------p8
			#    /|         /|
			#   +----------+ |
			#  /          /| |
			# p3--------p4 | |
			# |          | + |
			# |          |/-p6
			# +----------+  / 
			# | /        | /  
			# |/         |/
			# p1--------p2
			queue.append(Cube(p3.war, p4.war, round_to_multiple(p1.ws + ws_halfGap + decimal_episilon), p3.ws, p3.pb, round_to_multiple(p3.pb + pb_halfGap)))
		# si l'arête gauche/bas est homogène mais différente d'un point en dessus et à droite ET qu'on peut encore découper sur l'axe horizontal et vertical
		if (sol1 == sol5 and (sol1 != sol2 or sol5 != sol6) and (sol1 != sol3 or sol5 != sol7) and (war_halfGap >= decimal_episilon or ws_halfGap >= decimal_episilon)):
			# Construction d'un cube en bas à gauche
			#     p7--------p8
			#    /|         /|
			#   / |        / |
			#  /  +----+  /  |
			# p3--------p4   |
			# | /    / | |   |
			# |/    /  +-|--p6
			# +----+  /  |  / 
			# |    | /   | /  
			# |    |/    |/
			# p1---+----p2
			queue.append(Cube(p1.war, round_to_multiple(p1.war + war_halfGap), p1.ws, round_to_multiple(p1.ws + ws_halfGap), p1.pb, p5.pb))
		# si l'arête droite/bas est homogène mais différente d'un point en dessus et à gauche ET qu'on peut encore découper sur l'axe horizontal et vertical
		if (sol2 == sol6 and (sol1 != sol2 or sol5 != sol6) and (sol2 != sol4 or sol6 != sol8) and (war_halfGap >= decimal_episilon or ws_halfGap >= decimal_episilon)):
			# Construction d'un cube en bas à droite
			#     p7--------p8
			#    /|         /|
			#   / |        / |
			#  /  |     +-/--+
			# p3--------p4  /|
			# |   |   /  | / |
			# |   p5-/   |/ p6
			# |  /  +----+  / 
			# | /   |    | /  
			# |/    |    |/
			# p1----+---p2
			queue.append(Cube(round_to_multiple(p1.war + war_halfGap + decimal_episilon), p2.war, p2.ws, round_to_multiple(p2.ws + ws_halfGap), p2.pb, p6.pb))
		# si l'arête droite/haut est homogène mais différente d'un point en dessous et à gauche ET qu'on peut encore découper sur l'axe horizontal et vertical
		if (sol4 == sol8 and (sol4 != sol3 or sol8 != sol7) and (sol2 != sol4 or sol6 != sol8) and (war_halfGap >= decimal_episilon or ws_halfGap >= decimal_episilon)):
			# Construction d'un cube en haut à droite
			#     p7----+---p8
			#    /|    /    /|
			#   / |   /    / |
			#  /  |  /    /  +
			# p3----+---p4  /|
			# |   | |    | / |
			# |   p5|    |/-p6
			# |  /  +----+  / 
			# | /        | /  
			# |/         |/
			# p1--------p2
			queue.append(Cube(round_to_multiple(p3.war + war_halfGap + decimal_episilon), p4.war, round_to_multiple(p2.ws + ws_halfGap + decimal_episilon), p4.ws, p4.pb, p8.pb))
		# si l'arête gauche/haut est homogène mais différente d'un point en dessous et à droite ET qu'on peut encore découper sur l'axe horizontal et vertical
		if (sol3 == sol7 and (sol4 != sol3 or sol8 != sol7) and (sol1 != sol3 or sol5 != sol7) and (war_halfGap >= decimal_episilon or ws_halfGap >= decimal_episilon)):
			# Construction d'un cube en haut à gauche
			#     p7---+----p8
			#    /    /|    /|
			#   /    / |   / |
			#  /    /  +  /  |
			# p3---+----p4   |
			# |    | /   |   |
			# |    |/----|--p6
			# +----+     |  / 
			# | /        | /  
			# |/         |/
			# p1--------p2
			queue.append(Cube(p3.war, round_to_multiple(p3.war + war_halfGap), round_to_multiple(p1.ws + ws_halfGap + decimal_episilon), p3.ws, p3.pb, p7.pb))
		# si le coin avant/bas/gauche est isolé et qu'on peut encore découper sur au moins un des axes
		if(sol1 != sol2 and sol1 != sol3 and sol1 != sol5 and (war_halfGap >= decimal_episilon or ws_halfGap >= decimal_episilon or pb_halfGap >= decimal_episilon)):
			# Construction d'un cube en bas à gauche devant
			#     p7--------p8
			#    /|         /|
			#   / |        / |
			#  /  |       /  |
			# p3--------p4   |
			# | +----+   |   |
			# |/    /|---|--p6
			# +----+ |   |  /
			# |    | +   | /
			# |    |/    |/
			# p1---+----p2
			queue.append(Cube(p1.war, round_to_multiple(p1.war + war_halfGap), p1.ws, round_to_multiple(p1.ws + ws_halfGap), p1.pb, round_to_multiple(p1.pb + pb_halfGap)))
		# si le coin avant/haut/gauche est isolé et qu'on peut encore découper sur au moins un des axes
		if(sol3 != sol4 and sol3 != sol7 and sol3 != sol1 and (war_halfGap >= decimal_episilon or ws_halfGap >= decimal_episilon or pb_halfGap >= decimal_episilon)):
			# Construction d'un cube en haut à gauche devant
			#     p7--------p8
			#    /|         /|
			#   +----+     / |
			#  /    /|    /  |
			# p3---+----p4   |
			# |    | +   |   |
			# |    |/----|--p6
			# +----+     |  /
			# | /        | /
			# |/         |/
			# p1--------p2
			queue.append(Cube(p3.war, round_to_multiple(p3.war + war_halfGap), round_to_multiple(p1.ws + ws_halfGap + decimal_episilon), p3.ws, p3.pb, round_to_multiple(p3.pb + pb_halfGap)))
		# si le coin avant/bas/droite est isolé et qu'on peut encore découper sur au moins un des axes
		if(sol2 != sol1 and sol2 != sol6 and sol2 != sol4 and (war_halfGap >= decimal_episilon or ws_halfGap >= decimal_episilon or pb_halfGap >= decimal_episilon)):
			# Construction d'un cube en bas à droite devant
			#     p7--------p8
			#    /|         /|
			#   / |        / |
			#  /  |       /  |
			# p3--------p4   |
			# |   |   +--|-+ |
			# |   p5-/   |/|p6
			# |  /  +----+ |/
			# | /   |    | +
			# |/    |    |/
			# p1----+---p2
			queue.append(Cube(round_to_multiple(p1.war + war_halfGap + decimal_episilon), p2.war, p2.ws, round_to_multiple(p2.ws + ws_halfGap), p2.pb, round_to_multiple(p2.pb + pb_halfGap)))
		# si le coin avant/haut/droite est isolé et qu'on peut encore découper sur au moins un des axes
		if(sol4 != sol3 and sol4 != sol2 and sol4 != sol8 and (war_halfGap >= decimal_episilon or ws_halfGap >= decimal_episilon or pb_halfGap >= decimal_episilon)):
			# Construction d'un cube en haut à droite devant
			#     p7--------p8
			#    /|         /|
			#   / |   +----+ |
			#  /  |  /    /| |
			# p3----+---p4 | |
			# |   | |    | + |
			# |   p5|    |/-p6
			# |  /  +----+  /
			# | /        | /
			# |/         |/
			# p1--------p2
			queue.append(Cube(round_to_multiple(p3.war + war_halfGap + decimal_episilon), p4.war, round_to_multiple(p2.ws + ws_halfGap + decimal_episilon), p4.ws, p4.pb, round_to_multiple(p4.pb + pb_halfGap)))
		# si le coin arrière/bas/gauche est isolé et qu'on peut encore découper sur au moins un des axes
		if(sol5 != sol1 and sol5 != sol6 and sol5 != sol7 and (war_halfGap >= decimal_episilon or ws_halfGap >= decimal_episilon or pb_halfGap >= decimal_episilon)):
			# Construction d'un cube en bas à gauche derrière
			#     p7--------p8
			#    /|         /|
			#   / |        / |
			#  /  +----+  /  |
			# p3--------p4   |
			# | +----+ | |   |
			# | |    |-+-|--p6
			# | |    |/  |  /
			# | +----+   | /
			# |/         |/
			# p1--------p2
			queue.append(Cube(p5.war, round_to_multiple(p5.war + war_halfGap), p5.ws, round_to_multiple(p5.ws + ws_halfGap), round_to_multiple(p1.pb + pb_halfGap + decimal_episilon), p5.pb))
		# si le coin arrière/haut/gauche est isolé et qu'on peut encore découper sur au moins un des axes
		if(sol7 != sol3 and sol7 != sol8 and sol7 != sol5 and (war_halfGap >= decimal_episilon or ws_halfGap >= decimal_episilon or pb_halfGap >= decimal_episilon)):
			# Construction d'un cube en haut à gauche derrière
			#     p7---+----p8
			#    /    /|    /|
			#   +----+ |   / |
			#  /|    | +  /  |
			# p3--------p4   |
			# | +----+   |   |
			# |   p5-----|--p6
			# |  /       |  /
			# | /        | /
			# |/         |/
			# p1--------p2
			queue.append(Cube(p7.war, round_to_multiple(p7.war + war_halfGap), round_to_multiple(p5.ws + ws_halfGap + decimal_episilon), p7.ws, round_to_multiple(p3.pb + pb_halfGap + decimal_episilon), p7.pb))
		# si le coin arrière/bas/droite est isolé et qu'on peut encore découper sur au moins un des axes
		if(sol6 != sol5 and sol6 != sol2 and sol6 != sol8 and (war_halfGap >= decimal_episilon or ws_halfGap >= decimal_episilon or pb_halfGap >= decimal_episilon)):
			# Construction d'un cube en bas à droite derrière
			#     p7--------p8
			#    /|         /|
			#   / |        / |
			#  /  |     +-/--+
			# p3--------p4  /|
			# |   |   +--|-+ |
			# |   p5--|  | |p6
			# |  /    |  | |/
			# | /     +--|-+
			# |/         |/
			# p1--------p2
			queue.append(Cube(round_to_multiple(p5.war + war_halfGap + decimal_episilon), p6.war, p6.ws, round_to_multiple(p6.ws + ws_halfGap), round_to_multiple(p2.pb + pb_halfGap + decimal_episilon), p6.pb))
		# si le coin arrière/haut/droite est isolé et qu'on peut encore découper sur au moins un des axes
		if(sol8 != sol7 and sol8 != sol4 and sol8 != sol6 and (war_halfGap >= decimal_episilon or ws_halfGap >= decimal_episilon or pb_halfGap >= decimal_episilon)):
			# Construction d'un cube en haut à droite derrière
			#     p7----+---p8
			#    /|    /    /|
			#   / |   +----+ |
			#  /  |   |   /| +
			# p3--------p4 |/|
			# |   |   +--|-+ |
			# |   p5-----|--p6
			# |  /       |  /
			# | /        | /
			# |/         |/
			# p1--------p2
			queue.append(Cube(round_to_multiple(p7.war + war_halfGap + decimal_episilon), p8.war, round_to_multiple(p6.ws + ws_halfGap + decimal_episilon), p8.ws, round_to_multiple(p4.pb + pb_halfGap + decimal_episilon), p8.pb))


# \brief Compresse les logs contenus dans le fichier "targetFileName" en explorant les paramètres war et ws.
#
# @targetFileName : Nom du fichier log à compresser
# @l_solution : représente la solution de référence sous la forme d'une liste. Si égale à None le résultat de la compression n'est pas comparé à la solution.
def search_aveugle(targetFileName, l_solution=None):
	global  g_episilon, g_tab_parametersToBestResultPos, g_tab_parametersToColorId, g_map_bestSolutionToColorId, g_colorId
	num_etape = int(1+(1/g_episilon))
	g_tab_parametersToBestResultPos = np.zeros((num_etape, num_etape, num_etape))
	g_tab_parametersToColorId = np.zeros((num_etape, num_etape, num_etape))
	g_map_bestSolutionToColorId = {}
	g_colorId = 1
	if(not os.path.exists("./result/aveugle/"+targetFileName)):
		os.makedirs("./result/aveugle/"+targetFileName)
	else:
		for f in os.listdir("./result/aveugle/"+targetFileName):
			os.remove(os.path.join("./result/aveugle/"+targetFileName, f))
	# boucle pour calculer les war
	for i in range(num_etape):
		war = Decimal(i*g_episilon).quantize(Decimal('1.00')) # pour être sûr de n'avoir que deux chiffres après la virgule
		# boucle pour calculer les ws
		for j in range(num_etape):
			ws = Decimal(j*g_episilon).quantize(Decimal('1.00')) # pour être sûr de n'avoir que deux chiffres après la virgule
			# boucle pour calculer les pb
			for k in range(num_etape):
				pb = Decimal(k*g_episilon).quantize(Decimal('1.00')) # pour être sûr de n'avoir que deux chiffres après la virgule
				print("Call parser with parameters\twar: "+str(war)+"   \tws: "+str(ws)+"   \tpb: "+str(pb))
				# appel du parser C++
				file_exe = "myParserTest.exe "+ targetFileName+".log -sc 0.1 -war " +str(war) + " -ws "+str(ws)+" -pb "+str(pb)+" -disableLogs"
				child = subprocess.Popen(file_exe,shell=True)
				child.wait()
				# sauvegarde du résultat
				shutil.copyfile("./debugParser.log", "./result/aveugle/"+targetFileName+"/debugParser_"+targetFileName+"_war"+str(war) + "_ws"+str(ws) + "_pb"+str(pb)+".log")
				
				# cas où on compare le résultat à la solution optimale
				if(l_solution != None):
					g_tab_parametersToBestResultPos[i][j][k]=getBestResultPosEqualToSolution("./debugParser.log", l_solution)

				bestSolution = get_bestSolution("./debugParser.log")
				if(bestSolution in g_map_bestSolutionToColorId.keys()):
					g_tab_parametersToColorId[i][j][k] = g_map_bestSolutionToColorId[bestSolution]
				else:
					g_map_bestSolutionToColorId[bestSolution] = g_colorId
					g_tab_parametersToColorId[i][j][k] = g_colorId
					g_colorId += 1
	




# \brief Exécuter la recherche de paramètres avec la façon qu'on souhaite
#
# @dichotomique : utilisation l'algorithme de dichotomique avec des rectangles pour réduire le nombre de test
# @verification : True pour ajouter la partie de vérification de la solution avec une solution optimale, False pour sauter cette étape
def run(dichotomique=True, verification=False):
	global g_tab_parametersToBestResultPos, g_tab_parametersToColorId
	l_solution = None
	# Façon dichotomique
	if (dichotomique):
		for n in range(len(test_file)):
			targetFileName = test_file[n]
			print("Recherche des paramètres pour le fichier : "+targetFileName)
			if (verification):
				l_solution = tokenizeFile("./example/solutions/"+targetFileName+".log")
			search_war_ws_by_rect(targetFileName, l_solution)
			np.save("./files_npy/"+targetFileName+".npy",g_tab_parametersToBestResultPos)
			np.save("./files_npy/possible_"+targetFileName+".npy", g_tab_parametersToColorId)
			print("************************************************************\n\n")
	# Façon aveugle
	else:
		for n in range(len(test_file)):
			targetFileName = test_file[n]
			if (verification):
				l_solution = tokenizeFile("./example/solutions/"+targetFileName+".log")
			search_aveugle(targetFileName, l_solution)
			np.save("./files_npy/"+targetFileName+".npy",g_tab_parametersToBestResultPos)
			np.save("./files_npy/possible_"+targetFileName+".npy", g_tab_parametersToColorId)
			print("************************************************************\n\n")



if __name__ == "__main__":
	test_file = []
	argv = sys.argv
	if(not os.path.exists("./files_npy/")):
		os.makedirs("./files_npy/")
	if(len(argv)>2):
		test_file.append(argv[1])
		if(sys.argv[2]=="aveugle" and sys.argv[3]=="verification"):
			run(dichotomique=False,verification=True)
		elif(sys.argv[2]=="aveugle" and sys.argv[3]=="nonVerification"):
			run(dichotomique=False,verification=False)
		elif(sys.argv[2]=="dichotomique" and sys.argv[3]=="verification"):
			run(dichotomique=True,verification=True)
		elif(sys.argv[2]=="dichotomique" and sys.argv[3]=="nonVerification"):
			run(dichotomique=True,verification=False)
		else:
			print("Utilisation dans terminal : python rechercheParameters.py fileName aveugle/dichotimique verification/nonVerification\n")
	elif(len(sys.argv)==2):
		test_file.append(argv[1])
		run(dichotomique=True,verification=True)
	else:
		print("Lancement des tests...\n")
		test_file = ["1_rienAFaire", "2_simpleBoucle", "3_simpleBoucleAvecDebut", "4_simpleBoucleAvecFin", "5_simpleBoucleAvecDebutEtFin", "6.01_simpleBoucleAvecIf", "6.02_simpleBoucleAvecIf", "6.03_simpleBoucleAvecIf", "6.04_simpleBoucleAvecIf", "6.05_simpleBoucleAvecIf", "6.06_simpleBoucleAvecIf", "6.07_simpleBoucleAvecIf", "6.08_simpleBoucleAvecIf", "6.09_simpleBoucleAvecIf", "6.10_simpleBoucleAvecIf", "6.11_simpleBoucleAvecIf", "6.12_simpleBoucleAvecIf", "6.13_simpleBoucleAvecIf", "6.14_simpleBoucleAvecIf", "7.01_bouclesEnSequence", "7.02_bouclesEnSequence", "8_bouclesEnSequenceAvecIf", "9.01_bouclesImbriquees", "9.02_bouclesImbriquees", "9.03_bouclesImbriquees"]
		run(dichotomique=True,verification=True)
