--
-- List of constants for System units of Kernel Panic 3.1
--

with PP; use PP;

--------------
-- Units id --
--------------
package ConstantListKP31 is
	Assembler : constant PP.Kind := 2;
	BadBlock : constant PP.Kind := 3;
	Bit : constant PP.Kind := 4;
	Byte : constant PP.Kind := 7;
	Kernel : constant PP.Kind := 24;
	LogicBomb : constant PP.Kind := 25;
	Pointer : constant PP.Kind := 30;
	Signal : constant PP.Kind := 33;
	Socket : constant PP.Kind := 34;
	Terminal : constant PP.Kind := 35;
	
	----------------------------------------------------------------------------
	-- Orders available for ASSEMBLER, BIT, BYTE, KERNEL, LOGIC_BOMB, POINTER --
	-- and SOCKET                                                             --
	----------------------------------------------------------------------------
	Stop : constant PP.Action := 0;
	Wait : constant PP.Action := 5;
	FireState : constant PP.Action := 45;
	SelfDestruction : constant PP.Action := 65;
	Repeat : constant PP.Action := 115;
	---------------------------------------------------------------------------
	-- Orders available for ASSEMBLER, BIT, BYTE, KERNEL, POINTER and SOCKET --
	---------------------------------------------------------------------------
	Move : constant PP.Action := 10;
	Patrol : constant PP.Action := 15;
	Fight : constant PP.Action := 16;
	Guard : constant PP.Action := 25;
	MoveState : constant PP.Action := 50;
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

end ConstantListKP31;