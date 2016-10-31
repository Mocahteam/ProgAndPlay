with PP; use PP;
with ConstantListKP41; use ConstantListKP41;
with Ada.Text_IO; use Ada.Text_IO;

-- lancer l'attaque
procedure mission8_1 is
	-- trie l'ensemble des unites du joueur
	procedure trierUnites (raliementBytes : in Position; raliementBits : in Position) is
		cpt : Integer;
		u : Unit;
	begin
		cpt := 0;
		while cpt < GetNumUnits(MyCoalition) loop
			u := GetUnitAt(MyCoalition, cpt);
			if GetType(u) /= Byte then
				CarryOutCommand(u, Move, raliementBits);
			else
				CarryOutCommand(u, Move, raliementBytes);
			end if;
			cpt := cpt + 1;
		end loop;
	end trierUnites;
	
	-- attendre départ
	procedure attendreDepart is
		u : Unit;
		cpt : Integer;
		fini : Boolean;
	begin
		fini := False;
		while not fini loop
			cpt := 0;
			while cpt < GetNumUnits(MyCoalition) and not fini loop
				u := GetUnitAt(MyCoalition, cpt);
				if Integer(GetPendingCommands(u).Length) > 0 then
					fini := true;
				end if;
				cpt := cpt + 1;
			end loop;
		end loop;
	end attendreDepart;
	
	-- attendre la fin du rassemblement
	procedure attendreRassemblement is
		u : Unit;
		cpt : Integer;
		fini : Boolean;
	begin
		fini := False;
		while not fini loop
			fini := True;
			cpt := 0;
			while cpt < GetNumUnits(MyCoalition) and fini loop
				u := GetUnitAt(MyCoalition, cpt);
				if Integer(GetPendingCommands(u).Length) > 0 then
					fini := False;
				end if;
				cpt := cpt + 1;
			end loop;
		end loop;
	end attendreRassemblement;
	
	-- lancer l'attaque sur la position pos
	procedure lancerAttaque (pos : in Position) is
		i, j, urgent, urgencePrecedente, cpt : Integer;
		unite, assembleur : Unit;
		bytes : array (0..2) of Integer;
		asmTjrsVivant, asmNonTrouve : Boolean;
	begin
		-- recuperer le premier assembleur et les octets
		i := 0;
		cpt := 0;
		asmNonTrouve := True;
		while i < GetNumUnits(MyCoalition) loop
			unite := GetUnitAt(MyCoalition, i);
			if asmNonTrouve and GetType(unite) = Assembler then
				asmNonTrouve := False;
				assembleur := unite;
			end if;
			if cpt < bytes'Length and GetType(unite) = Byte then
				bytes(cpt) := unite;
				cpt := cpt + 1;
			end if;
			i := i + 1;
		end loop;
		-- lancer la marche
		i := 0;
		while i < GetNumUnits(MyCoalition) loop
			unite := GetUnitAt(MyCoalition, i);
			if GetType(unite) = Byte then
				CarryOutCommand(unite, Fight, pos);
			else
				CarryOutCommand(unite, Guard, bytes(0));
			end if;
			i := i + 1;
		end loop;
		-- avancer jusqu'a trouver l'ennemie
		while GetNumUnits(EnemyCoalition) = 0 loop
			Put_Line ("Avancer jusqu'a trouver l'ennemi");
		end loop;
		-- tout lacher (sauf assembleur)
		i := 0;
		while i < GetNumUnits(MyCoalition) loop
			unite := GetUnitAt(MyCoalition, i);
			if GetType(unite) /= Assembler then
				CarryOutCommand(unite, Fight, pos);
			end if;
			i := i + 1;
		end loop;
		-- reparer les bytes endommages
		urgencePrecedente := -1;
		asmTjrsVivant := GetType(assembleur) > 0;
		while not IsGameOver and asmTjrsVivant loop
			-- tester si l'assembleur est toujours vivant
			asmTjrsVivant := GetType(assembleur) > 0;
			if asmTjrsVivant then
				-- supprimer les octets détruits
				i := 0;
				while i < cpt loop
					if GetType(bytes(i)) = -1 then
						-- supprimer cet octet
						j := i;
						while j+1 < cpt loop
							bytes(j) := bytes(j+1);
							j := j + 1;
						end loop;
						cpt := cpt - 1;
					else
						i := i + 1;
					end if;
				end loop;
				-- recherche de l'octet le plus endommage
				i := 0;
				urgent := -1;
				while i < cpt loop
					-- verifier si l'octet courant est plus prioritaire que la precedente
					-- urgence
					if urgent = -1 then
						urgent := i;
					end if;
					if urgent /= i then
						if GetHealth(bytes(i)) < GetHealth(bytes(urgent)) then
							urgent := i;
						end if;
					end if;
					i := i + 1;
				end loop;
				if urgent /= -1 then
					if urgent /= urgencePrecedente or Integer(GetPendingCommands(assembleur).Length) = 0 then
						CarryOutCommand(assembleur, Repair, bytes(urgent));
						urgencePrecedente := urgent;
					end if;
				end if;
			end if;
		end loop;
	end lancerAttaque;
	
	attaque, bytes, bits : Position;
	
begin
	Open; -- ouverture du jeu
	-- initialisation des positions
	attaque.x := 1792.0;
	attaque.y := 256.0;
	bytes.x := 478.0;
	bytes.y := 255.0;
	bits.x := 255.0;
	bits.y := 255.0;
	
	trierUnites(bytes, bits);
	
	attendreDepart;
	
	attendreRassemblement;
	
	lancerAttaque (attaque);
	
	Close; -- fermer le jeu
end mission8_1;