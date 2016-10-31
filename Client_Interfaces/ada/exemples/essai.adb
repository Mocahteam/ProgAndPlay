with Text_Io; use Text_Io;
with PP; use PP;
with ConstantListKP41; use ConstantListKP41;
with Ada.Containers; use Ada.Containers;

procedure essai is
	PACKAGE Es_Entiers IS NEW Integer_IO(Integer);
	PACKAGE Es_Counter IS NEW Integer_IO(Count_Type);
	PACKAGE ES_Bool IS NEW Enumeration_IO(Boolean);
	PACKAGE ES_Coalition IS NEW Enumeration_IO(Coalition);
	PACKAGE Es_Reels IS NEW Float_IO(Float);
	p : Position;
	u : Unit;
	pdgCmds : Cmd_Container.Vector;
begin
	Open;
	-- ES_Bool.put(IsGameOver);
	-- p := GetMapSize;
	-- if (p.x = -1.0) then
		-- PrintError;
	-- else
		-- Es_Reels.put(p.x);
		-- Es_Reels.put(p.y);
	-- end if;
	-- p := GetStartPosition;
	-- Es_Reels.put(p.x);
	-- Es_Reels.put(p.y);
	-- Es_Entiers.put(GetNumSpecialAreas);
	-- p := GetSpecialAreaPosition(0);
	-- Es_Reels.put(p.x);
	-- Es_Reels.put(p.y);
	-- Es_Entiers.put(GetNumUnits(MyCoalition));
	u := GetUnitAt(MyCoalition, 0);
	-- ES_Coalition.put(GetCoalition(u));
	-- Es_Entiers.put(GetType(u));
	-- p := GetPosition (u);
	-- Es_Reels.put(p.x);
	-- Es_Reels.put(p.y);
	-- Es_Reels.put(GetHealth(u));
	-- Es_Reels.put(GetMaxHealth(u));
	pdgCmds := GetPendingCommands(u);
	Es_Counter.put(pdgCmds.Length);
	for i in pdgCmds.First_Index .. pdgCmds.Last_Index loop
		Es_Entiers.put(pdgCmds.Element(i).code);
		for j in pdgCmds.Element(i).param.First_Index .. pdgCmds.Element(i).param.Last_Index loop
			Es_Reels.put(pdgCmds.Element(i).param.Element(j));
		end loop;
	end loop;
	-- Es_Entiers.put(GetGroup(u));
	-- SetGroup(u, 1);
	-- p.x := 100.0;
	-- p.y := 100.0;
	-- CarryOutCommand(u, BuildBit, p);
	-- CarryOutCommand(GetUnitAt(MyCoalition, 1), Guard, u);
	-- CarryOutCommand(GetUnitAt(MyCoalition, 1), Move, p);
	-- CarryOutCommand(GetUnitAt(MyCoalition, 1), Stop, -1.0);
	CarryOutCommand(u, StopBuiding, -1.0);
	
	Close;
end essai;