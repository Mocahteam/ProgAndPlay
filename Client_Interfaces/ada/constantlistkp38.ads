--
-- List of constants for System units of Kernel Panic 3.8
--

with PP; use PP;

package ConstantListKP38 is
	--------------
	-- Units id --
	--------------
	Assembler : constant PP.Kind := 2;
	BadBlock : constant PP.Kind := 3;
	Bit : constant PP.Kind := 4;
	Byte : constant PP.Kind := 7;
	Kernel : constant PP.Kind := 24;
	LogicBomb : constant PP.Kind := 25;
	Pointer : constant PP.Kind := 37;
	Signal : constant PP.Kind := 40;
	Socket : constant PP.Kind := 41;
	Terminal : constant PP.Kind := 42;
	
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
	Attack : constant PP.Action := 20;
	------------------------------------
	-- Orders available for ASSEMBLER --
	------------------------------------
	Repair : constant PP.Action := 40;
	Reclaim : constant PP.Action := 90;
	Restore : constant PP.Action := 110;
	-------------------------------
	-- Orders available for BYTE --
	-------------------------------
	LaunchMine : constant PP.Action := 33395; 
	----------------------------------
	-- Orders available for POINTER --
	----------------------------------
	NxFlafg : constant PP.Action := 33389;
	-----------------------------------
	-- Orders available for TERMINAL --
	-----------------------------------
	Sigterm : constant PP.Action := 35126;
	
	------------------
	-- Resources id --
	------------------
	Metal : constant PP.Resource := 0;
	Energy : constant PP.Resource := 1;

end ConstantListKP38;