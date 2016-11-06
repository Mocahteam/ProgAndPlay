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
			trace ("Windows : Tentative d'ouverture de l'extension de nom ProgAndPlayExtension");
			progPlayContext = ExtensionContext.createExtensionContext("ProgAndPlayExtension", "");
			if (progPlayContext == null)
				trace ("Extension non chargée...");
			else
				trace ("Extension chargée (YES!!!) ...");
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
		
		public function PP_GetUnitAt_ext(coalition:int, id:int):int{
			if (progPlayContext != null)
				return int (progPlayContext.call("PP_GetUnitAt_wrapper", coalition, id));
			return -1;
		}
		
		public function PP_Unit_GetCoalition_ext(unitId:int):int{
			if (progPlayContext != null)
				return int (progPlayContext.call("PP_Unit_GetCoalition_wrapper", unitId));
			return -1;
		}
		
		public function PP_Unit_GetType_ext(unitId:int):int{
			if (progPlayContext != null)
				return int (progPlayContext.call("PP_Unit_GetType_wrapper", unitId));
			return -1;
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
		
		public function PP_Close_ext():void{
			if (progPlayContext != null)
				progPlayContext.call("PP_Close_wrapper");
		}
	}
}