with Ada.Text_IO;  use Ada.Text_Io;
with Interfaces.C.Strings;

package body PP is
	function GetPendingCommands (u : in Unit) return Cmd_Container.Vector is
		i, j, ret : Integer;
		cmdCode : aliased Integer;
		paramValue : aliased Float;
		cmd : Command;
		pendingCmd : Cmd_Container.Vector;
	begin
		enterCriticalSection;
			pendingCmd.Clear;
			i := 0;
			ret := 0;
			while i < PP_Unit_GetNumPdgCmds_prim(u) and ret >= 0 loop
				ret := PP_Unit_PdgCmd_GetCode_prim(u, i, cmdCode'Access);
				if ret = 0 then
					cmd.code := cmdCode;
					cmd.param.Clear;
					j := 0;
					ret := PP_Unit_PdgCmd_GetNumParams_prim(u, i);
					while j < PP_Unit_PdgCmd_GetNumParams_prim(u, i) and ret >= 0 loop
						ret := PP_Unit_PdgCmd_GetParam_prim(u, i, j, paramValue'Access);
						if ret = 0 then
							cmd.param.Append(paramValue);
						end if;
						j := j + 1;
					end loop;
					pendingCmd.Append(cmd);
				end if;
				i := i + 1;
			end loop;
		exitCriticalSection;
		return pendingCmd;
	end;

	procedure CarryOutCommand (u : in Unit; c : in CommandCode; t : in Unit; s : in Boolean := False) is
	begin
		PP_Unit_ActionOnUnit(u, c, t, s);
	end;

	procedure CarryOutCommand (u : in Unit; c : in CommandCode; t : in Position; s : in Boolean := False) is
	begin
		Ada_ActionOnPosition (u, c, t.x, t.y, s);
	end;

	procedure CarryOutCommand (u : in Unit; c : in CommandCode; p : in Float := -1.0; s : in Boolean := False) is
	begin
		PP_Unit_UntargetedAction (u, c, p, s);
	end;
	
	procedure PrintError is
	begin
		Ada.Text_io.put(Interfaces.C.Strings.Value(PP_GetError));
		PP_ClearError;
	end;
end PP;