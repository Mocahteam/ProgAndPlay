with Ada.Text_IO;  use Ada.Text_Io;
with Interfaces.C.Strings;

package body PP is
	function GetPendingCommands (u : in Unit) return Cmd_Container.Vector is
		i, j, ret : Integer;
		retF: Float;
		cmd : Command;
		pendingCmd : Cmd_Container.Vector;
	begin
		enterCriticalSection;
			pendingCmd.Clear;
			i := 0;
			ret := GetNumPdgCmds(u);
			while i < GetNumPdgCmds(u) and ret /= -1 loop
				ret := PdgCmd_GetCode(u, i);
				if ret /= -1 then
					cmd.code := ret;
					cmd.param.Clear;
					j := 0;
					ret := PdgCmd_GetNumParams(u, i);
					while j < PdgCmd_GetNumParams(u, i) and ret /= -1 loop
						retF := PdgCmd_GetParam(u, i, j);
						if retF /= -1.0 then
							cmd.param.Append(retF);
						else
							ret := -1;
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

	procedure CarryOutCommand (u : in Unit; c : in CommandCode; t : in Unit) is
	begin
		ActionOnUnit(u, c, t);
	end;

	procedure CarryOutCommand (u : in Unit; c : in CommandCode; t : in Position) is
	begin
		ActionOnPosition (u, c, t.x, t.y);
	end;

	procedure CarryOutCommand (u : in Unit; c : in CommandCode; p : in Float := -1.0) is
	begin
		UntargetedAction (u, c, p);
	end;
	
	procedure PrintError is
	begin
		Ada.Text_io.put(Interfaces.C.Strings.Value(GetError));
		ClearError;
	end;
end PP;