--
-- List of constants for System units of Kernel Panic 4.1
--

with PP; use PP;

package ConstantListKP41 is
	--------------
	-- Units id --
	--------------
	Assembler : constant PP.Kind := 2;
	BadBlock : constant PP.Kind := 3;
	Bit : constant PP.Kind := 4;
	Byte : constant PP.Kind := 7;
	Kernel : constant PP.Kind := 25;
	LogicBomb : constant PP.Kind := 26;
	Pointer : constant PP.Kind := 39;
	Signal : constant PP.Kind := 44;
	Socket : constant PP.Kind := 45;
	Terminal : constant PP.Kind := 46;
	
	----------------------------------------------------------------------------
	-- Orders available for ASSEMBLER, BIT, BYTE, KERNEL, LOGIC_BOMB, POINTER --
	-- and SOCKET                                                             --
	----------------------------------------------------------------------------
	Stop : constant PP.Action := 0;             -- expect 0 parameters
	Wait : constant PP.Action := 5;             -- expect 0 parameters
	FireState : constant PP.Action := 45;       -- expect 1 parameter:
	                                            --     0.0 => Hold fire
	                                            --     1.0 => Return fire
	                                            --     2.0 => Fire at will
	SelfDestruction : constant PP.Action := 65; -- expect 0 parameters
	Repeat : constant PP.Action := 115;         -- expect 1 parameter:
	                                            --     0.0 => Repeat off
	                                            --     1.0 => Repeat on
	---------------------------------------------------------------------------
	-- Orders available for ASSEMBLER, BIT, BYTE, KERNEL, POINTER and SOCKET --
	---------------------------------------------------------------------------
	Move : constant PP.Action := 10;          -- expect 1 parameter:
	                                          --       a position or a unit
	Patrol : constant PP.Action := 15;        -- expect 1 parameter:
	                                          --       a position or a unit
	Fight : constant PP.Action := 16;         -- expect 1 parameter:
	                                          --       a position or a unit
	Guard : constant PP.Action := 25;         -- expect 1 parameter:
	                                          --       a position or a unit
	MoveState : constant PP.Action := 50;     -- expect 1 parameter:
	                                          --       0.0 => Hold pos
	                                          --       1.0 => Maneuver
	                                          --       2.0 => Roam
	----------------------------------------------------------------------------
	-- Orders available for BIT, BYTE, KERNEL, LOGIC_BOMB, POINTER and SOCKET --
	----------------------------------------------------------------------------
	Attack : constant PP.Action := 20;  -- expect 1 parameter: a position or a unit
	
	------------------------------------
	-- Orders available for ASSEMBLER --
	------------------------------------
	Repair : constant PP.Action := 40;          -- expect 1 parameter: a position or a unit
	Reclaim : constant PP.Action := 90;         -- expect 1 parameter: a position or a unit
	Restore : constant PP.Action := 110;        -- expect 1 parameter: a position or a unit
	BuildBadBlock : constant PP.Action := -3;   -- expect 1 parameter: a position or a unit
	BuildLogicBomb : constant PP.Action := -26; -- expect 1 parameter: a position or a unit
	BuildSocket : constant PP.Action := -45;    -- expect 1 parameter: a position or a unit
	Buildterminal : constant PP.Action := -46;  -- expect 1 parameter: a position or a unit
	Debug : constant PP.Action := -35;          -- expect 1 parameter: a position or a unit
	
	---------------------------------
	-- Orders available for KERNEL --
	---------------------------------
	BuildAssembler : constant PP.Action := -2; -- expect 1 parameter: a position or a unit
	BuildByte : constant PP.Action := -7;      -- expect 1 parameter: a position or a unit
	BuildPointer : constant PP.Action := -39;  -- expect 1 parameter: a position or a unit
	
	--------------------------------------------
	-- Orders available for KERNEL and SOCKET --
	--------------------------------------------
	BuildBit : constant PP.Action := -4;       -- expect 1 parameter: a position or a unit
	StopBuiding : constant PP.Action := -7658; -- expect 0 parameters
	
	-------------------------------
	-- Orders available for BYTE --
	-------------------------------
	LaunchMine : constant PP.Action := 33395; -- expect 0 parameters
	----------------------------------
	-- Orders available for POINTER --
	----------------------------------
	NxFlafg : constant PP.Action := 33389; -- expect 1 parameter: a position or a unit
	-----------------------------------
	-- Orders available for TERMINAL --
	-----------------------------------
	Sigterm : constant PP.Action := 35126; -- expect 1 parameter: a position or a unit
	
	------------------
	-- Resources id --
	------------------
	Metal : constant PP.Resource := 0;
	Energy : constant PP.Resource := 1;

end ConstantListKP41;