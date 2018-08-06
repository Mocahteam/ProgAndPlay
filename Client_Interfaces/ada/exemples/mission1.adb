with PP; use PP;
with ConstantListKP47; use ConstantListKP47;
with Text_Io; use Text_Io;

-- deplacer une unite a une position precise
procedure mission1 is
	PACKAGE Es_Reels IS NEW Float_IO(Float);
	p : Position; -- position a atteindre
	u : Unit; -- unite a deplacer
begin
	Open; -- ouverture du jeu
	-- definir la position
	p.x := 1980.0;
	p.y := 1676.0;
	-- recuperer l'unite
	u := GetUnitAt(MyCoalition, 0);
	-- deplacer l'unite sur sa position
	CarryOutCommand(u, Move, p);
	Close; -- fermer le jeu
end mission1;