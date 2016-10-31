with PP; use PP;
with ConstantListKP41; use ConstantListKP41;
with Ada.Text_IO; use Ada.Text_IO;

-- reparer toutes les unites endommagees
procedure mission7 is
	u, assembleur : Unit; -- unites de traitement
	cpt : Integer; -- compteur de boucle
begin
	Open; -- ouverture du jeu
	-- recherche de l'assembleur
	cpt := 0;
	u := GetUnitAt(MyCoalition, cpt);
	while cpt < GetNumUnits(MyCoalition) and GetType(u) /= Assembler loop
		-- passer a l'unite suivante
		cpt := cpt+1;
		u := GetUnitAt(MyCoalition, cpt);
	end loop;
	if GetType(u) = Assembler then
		assembleur := u;
		-- reparation des unites
		cpt := 0;
		while cpt < GetNumUnits(MyCoalition) loop
			-- recupere l'unite courante
			u := GetUnitAt(MyCoalition, cpt);
			if GetHealth(u) < GetMaxHealth(u) then
				-- reparer l'unite
				CarryOutCommand(assembleur, Repair, u);
				-- attendre d'avoir fini de la reparer
				while GetHealth(u) < GetMaxHealth(u) loop
					Put_Line("Sante de l'unite " & Integer'Image(u) & " : " & Float'Image(GetHealth(u)) & "/" & Float'Image(GetMaxHealth(u)));
				end loop;
			end if;
			cpt := cpt + 1;
		end loop;
	end if;
	Close; -- fermer le jeu
end mission7;