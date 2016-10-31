with PP; use PP;
with ConstantListKP41; use ConstantListKP41;
with Ada.Text_IO; use Ada.Text_IO;

-- Defendre l'allie
procedure mission3 is
	u : Unit; -- mon unite
	e : Unit; -- unite a attaquer
begin
	Open; -- ouverture du jeu
	-- recuperer mon unite
	u := GetUnitAt(MyCoalition, 0);
	-- attendre l'unite ennemie
	while GetNumUnits(EnemyCoalition) = 0 loop
		Put_Line ("En attente de l'arrivée de l'ennemi");
	end loop;
	-- recuperer l'unite ennemie
	e := GetUnitAt(EnemyCoalition, 0);
	-- lancer l'attaque
	CarryOutCommand(u, Attack, e);
	Close; -- fermer le jeu
end mission3;