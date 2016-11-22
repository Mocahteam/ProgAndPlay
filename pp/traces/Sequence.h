/**
 * \file Sequence.h
 * \brief Déclaration de la classe Sequence
 * \author meresse
 * \version 0.1
 */

#ifndef __SEQUENCE_H__
#define __SEQUENCE_H__

#include <iostream>
#include <sstream>
#include <stack>
#include <vector>
#include <algorithm>
#include <map>
#include <cmath>
#include <boost/lexical_cast.hpp>
#include <boost/enable_shared_from_this.hpp>

#include "Trace.h"
#include "Call.h"

/** 
 * \class Sequence
 * \brief La classe Sequence hérite de la classe @see Trace. Une séquence possède un vecteur d'objets Trace.
 */
class Sequence : public Trace, public boost::enable_shared_from_this<Sequence> {
	
public:

	typedef boost::shared_ptr<Sequence> sp_sequence;
	typedef boost::shared_ptr<const Sequence> const_sp_sequence;
	typedef std::vector<sp_sequence> sequence_vector;
	
	Sequence(std::string info, bool num_fixed);
	Sequence(unsigned int num, bool root = false);
	Sequence(const_sp_sequence sps);
	Sequence(const_sp_sequence sps_up, const_sp_sequence sps_down);
	
	virtual unsigned int length() const;
	virtual bool operator==(Trace *t) const;
	virtual Trace::sp_trace clone() const;
	virtual void display(std::ostream &os = std::cout) const;
	virtual void resetAligned();
	bool compare(Trace *t);
	
	template<typename T>
	static std::string getNumMapString(const std::map<unsigned int,T>& numMap) {
		std::stringstream ss;
		typename std::map<unsigned int,T>::const_iterator it = numMap.begin();
		while (it != numMap.end()) {
			if (it != numMap.begin())
				ss << " ";
			ss << it->first << ":" << it->second;
			it++;
		}
		return ss.str();
	}
	
	std::vector<Trace::sp_trace>& getTraces();
	Call::call_vector getCalls(bool setMod = false);
	sequence_vector getSequences();
	int getIndex(const Trace::sp_trace& spt) const;
	unsigned int getNum() const;
	bool hasNumberIterationFixed() const;
	const std::map<unsigned int,unsigned int>& getNumMap() const;
	unsigned int getPt() const;
	bool isEndReached() const;
	bool isShared() const;
	bool isRoot() const;
	void addOne();
	unsigned int size() const;
	bool addTrace(Trace::sp_trace spt, int ind = -1);
	const Trace::sp_trace& at(unsigned int i) const;
	const Trace::sp_trace& next();
	void reset();
	
	bool isValid() const;
	void setValid(bool v);
	bool checkValid();
	bool isUniform() const;
	bool checkDelayed();
	
	void updateNumMap(unsigned int num, int update = 1);
	void updateNumMap(const std::map<unsigned int,unsigned int>& numMap);
	void completeNumMap(const sp_sequence& sps);
	std::map<unsigned int,double> getPercentageNumMap() const;
	double getNumMapMeanDistance(const sp_sequence& sps) const;
	bool isImplicit();
	
protected:
	
	/// Un booléen permettant d'indiquer si le nombre d'occurrences de la sequence doit être utilisé lors de l'analyse pour le calcul du score du joueur et pour la détermination des feedbacks. Cette variable est mise à vraie uniquement si l'expert a fixé la valeur de l'attribut 'nb_iteration_fixed' à 'true' pour la séquence dans le fichier XML utilisé pour l'import.
	bool num_fixed;
	
	/// Le vecteur des traces de la séquence.
	std::vector<Trace::sp_trace> traces;
	
	/// Un entier indiquant le nombre d'occurrences actuel de la séquence.
	unsigned int num;
	
	/// Permet de savoir comment la séquence se répète. Une entrée x:y signifie que la séquence de traces se répète y fois avec un nombre de répétitions contiguës égal à x.
	std::map<unsigned int,unsigned int> numMap;
	
	/// Un indice sur le vecteur des traces de la séquence. A chaque appel à Sequence::next, cette valeur est incrémentée et le prochain élément du vecteur est renvoyé.
	unsigned int pt;
	
	/// [deprecated - online version]
	bool valid;
	
	/// Un booléen mis à vrai lorsque la dernière trace du vecteur de la séquence a été renvoyée suite à l'appel de Sequence::next.
	bool endReached;
	
	/// Un booléen utilisé uniquement lors de la fusion de deux séquences TracesParser::mergeSequences.
	bool shared;
	
	bool root;
	
};

#endif