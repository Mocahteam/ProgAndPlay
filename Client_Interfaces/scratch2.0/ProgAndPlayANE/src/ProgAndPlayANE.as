package
{

	import flash.events.EventDispatcher;
	import flash.external.ExtensionContext;
	/**
	 * ...
	 * @author MM
	 */
	public class ProgAndPlayANE extends EventDispatcher
	{
		
		private var progPlayContext:ExtensionContext = null;
		
		public function ProgAndPlayANE() 
		{
			trace ("Windows : Try to open ProgAndPlayExtension");
			progPlayContext = ExtensionContext.createExtensionContext("ProgAndPlayExtension", "");
			if (progPlayContext == null)
				trace ("Extension not loaded...");
			else
				trace ("Extension loaded...");
		}
		
		public function PP_Open_ext():void{
			if (progPlayContext != null)
				progPlayContext.call("PP_Open_wrapper");
		}
		
		public function PP_IsGameOver_ext():Boolean{
			if (progPlayContext != null)
				return Boolean (progPlayContext.call("PP_IsGameOver_wrapper"));
			return false;
		}
		
		public function PP_IsGamePaused_ext():Boolean{
			if (progPlayContext != null)
				return Boolean (progPlayContext.call("PP_IsGamePaused_wrapper"));
			return false;
		}
		
		public function PP_GetMapWidth_ext():int{
			if (progPlayContext != null)
				return int (progPlayContext.call("PP_GetMapWidth_wrapper"));
			return -1;
		}
		
		public function PP_GetMapHeight_ext():int{
			if (progPlayContext != null)
				return int (progPlayContext.call("PP_GetMapHeight_wrapper"));
			return -1;
		}
		
		public function PP_GetStartPosX_ext():Number{
			if (progPlayContext != null)
				return Number (progPlayContext.call("PP_GetStartPosX_wrapper"));
			return -1;
		}
		
		public function PP_GetStartPosY_ext():Number{
			if (progPlayContext != null)
				return Number (progPlayContext.call("PP_GetStartPosY_wrapper"));
			return -1;
		}
		
		public function PP_GetNumSpecialArea_ext():int{
			if (progPlayContext != null)
				return int (progPlayContext.call("PP_GetNumSpecialArea_wrapper"));
			return -1;
		}
		
		public function PP_GetSpecialAreaPosX_ext(num:int):Number{
			if (progPlayContext != null)
				return Number (progPlayContext.call("PP_GetSpecialAreaPosX_wrapper", num));
			return -1;
		}
		
		public function PP_GetSpecialAreaPosY_ext(num:int):Number{
			if (progPlayContext != null)
				return Number (progPlayContext.call("PP_GetSpecialAreaPosY_wrapper", num));
			return -1;
		}
		
		public function PP_GetResource_ext(id:int):int{
			if (progPlayContext != null)
				return int (progPlayContext.call("PP_GetResource_wrapper", id));
			return -1;
		}
		
		public function PP_GetNumUnits_ext(coalition:int):int{
			if (progPlayContext != null)
				return int (progPlayContext.call("PP_GetNumUnits_wrapper", coalition));
			return -1;
		}
		
		public function PP_GetUnitAt_ext(id:int, coalition:int):int{
			if (progPlayContext != null)
				return int (progPlayContext.call("PP_GetUnitAt_wrapper", id, coalition));
			return -1;
		}
		
		public function PP_Unit_BelongTo_ext(unitId:int, coalition:int):Boolean{
			if (progPlayContext != null)
				return Boolean (progPlayContext.call("PP_Unit_BelongTo_wrapper", unitId, coalition));
			return false;
		}
		
		public function PP_Unit_IsType_ext(unitId:int, unitType:int):Boolean{
			if (progPlayContext != null)
				return Boolean (progPlayContext.call("PP_Unit_IsType_wrapper", unitId, unitType));
			return false;
		}
		
		public function PP_Unit_GetPositionX_ext(unitId:int):Number{
			if (progPlayContext != null)
				return Number (progPlayContext.call("PP_Unit_GetPositionX_wrapper", unitId));
			return -1;
		}
		
		public function PP_Unit_GetPositionY_ext(unitId:int):Number{
			if (progPlayContext != null)
				return Number (progPlayContext.call("PP_Unit_GetPositionY_wrapper", unitId));
			return -1;
		}
		
		public function PP_Unit_GetHealth_ext(unitId:int):Number{
			if (progPlayContext != null)
				return Number (progPlayContext.call("PP_Unit_GetHealth_wrapper", unitId));
			return -1;
		}
		
		public function PP_Unit_GetMaxHealth_ext(unitId:int):Number{
			if (progPlayContext != null)
				return Number (progPlayContext.call("PP_Unit_GetMaxHealth_wrapper", unitId));
			return -1;
		}
		
		public function PP_Unit_GetGroup_ext(unitId:int):int{
			if (progPlayContext != null)
				return int (progPlayContext.call("PP_Unit_GetGroup_wrapper", unitId));
			return -1;
		}
		
		public function PP_Unit_SetGroup_ext(unitId:int, groupId:int):void{
			if (progPlayContext != null)
				progPlayContext.call("PP_Unit_SetGroup_wrapper", unitId, groupId);
		}
		
		public function PP_Unit_GetNumPendingCmds_ext(unitId:int):int{
			if (progPlayContext != null)
				return int (progPlayContext.call("PP_Unit_GetNumPendingCmds_wrapper", unitId));
			return -1;
		}
		
		public function PP_Unit_PdgCmd_IsEqualTo_ext(cmdId:int, unitId:int, cmdCodeToTest:int):Boolean{
			if (progPlayContext != null)
				return Boolean (progPlayContext.call("PP_Unit_PdgCmd_IsEqualTo_wrapper", cmdId, unitId, cmdCodeToTest));
			return false;
		}
		
		public function PP_Unit_PdgCmd_GetNumParams_ext(cmdId:int, unitId:int):int{
			if (progPlayContext != null)
				return int (progPlayContext.call("PP_Unit_PdgCmd_GetNumParams_wrapper", cmdId, unitId));
			return -1;
		}
		
		public function PP_Unit_PdgCmd_GetParamAt_ext(paramId:int, cmdId:int, unitId:int):Number{
			if (progPlayContext != null)
				return Number (progPlayContext.call("PP_Unit_PdgCmd_GetParamAt_wrapper", paramId, cmdId, unitId));
			return -1;
		}
		
		public function PP_Unit_ActionOnPosition_ext(unitId:int, cmdId:int, x:Number, y:Number, synchro:int):void{
			if (progPlayContext != null)
				progPlayContext.call("PP_Unit_ActionOnPosition_wrapper", unitId, cmdId, x, y, synchro);
		}
		
		public function PP_Unit_ActionOnUnit_ext(unitId:int, cmdId:int, targetId:int, synchro:int):void{
			if (progPlayContext != null)
				progPlayContext.call("PP_Unit_ActionOnUnit_wrapper", unitId, cmdId, targetId, synchro);
		}
		
		public function PP_Unit_UntargetedAction_ext(unitId:int, cmdId:int, param:Number, synchro:int):void{
			if (progPlayContext != null)
				progPlayContext.call("PP_Unit_UntargetedAction_wrapper", unitId, cmdId, param, synchro);
		}
		
		public function PP_Unit_UntargetedActionWithoutParam_ext(unitId:int, cmdId:int, synchro:int):void{
			if (progPlayContext != null)
				progPlayContext.call("PP_Unit_UntargetedActionWithoutParam_wrapper", unitId, cmdId, synchro);
		}
		
		public function PP_Close_ext():void{
			if (progPlayContext != null)
				progPlayContext.call("PP_Close_wrapper");
		}
		
		public function PP_GetError_ext():String{
			if (progPlayContext != null)
				return String (progPlayContext.call("PP_GetError_wrapper"));
			return "ProgAndPlayExtension not loaded";
		}
		
		public function PP_ClearError_ext():void{
			if (progPlayContext != null)
				progPlayContext.call("PP_ClearError_wrapper");
		}
	}
}