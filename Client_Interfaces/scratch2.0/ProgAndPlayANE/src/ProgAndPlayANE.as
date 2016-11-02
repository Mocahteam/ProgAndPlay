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
		
		public function PP_MapWidth_ext():int{
			if (progPlayContext != null)
				return int (progPlayContext.call("PP_MapWidth_wrapper"));
			return -1;
		}
		
		public function PP_MapHeight_ext():int{
			if (progPlayContext != null)
				return int (progPlayContext.call("PP_MapHeight_wrapper"));
			return -1;
		}
		
		public function PP_StartPosX_ext():Number{
			if (progPlayContext != null)
				return Number (progPlayContext.call("PP_StartPosX_wrapper"));
			return -1;
		}
		
		public function PP_StartPosY_ext():Number{
			if (progPlayContext != null)
				return Number (progPlayContext.call("PP_StartPosY_wrapper"));
			return -1;
		}
		
		public function PP_NumSpecialArea_ext():int{
			if (progPlayContext != null)
				return int (progPlayContext.call("PP_NumSpecialArea_wrapper"));
			return -1;
		}
		
		public function PP_SpecialAreaPosX_ext(num:int):Number{
			if (progPlayContext != null)
				return int (progPlayContext.call("PP_SpecialAreaPosX_wrapper", num));
			return -1;
		}
		
		public function PP_SpecialAreaPosY_ext(num:int):Number{
			if (progPlayContext != null)
				return int (progPlayContext.call("PP_SpecialAreaPosY_wrapper", num));
			return -1;
		}
		
		public function PP_Resource_ext(id:int):int{
			if (progPlayContext != null)
				return int (progPlayContext.call("PP_Resource_wrapper", id));
			return -1;
		}
		
		public function PP_NumUnits_ext(coalition:int):int{
			if (progPlayContext != null)
				return int (progPlayContext.call("PP_NumUnits_wrapper", coalition));
			return -1;
		}
		
		public function PP_UnitAt_ext(coalition:int, id:int):int{
			if (progPlayContext != null)
				return int (progPlayContext.call("PP_UnitAt_wrapper", coalition, id));
			return -1;
		}
		
		public function PP_Close_ext():void{
			if (progPlayContext != null)
				progPlayContext.call("PP_Close_wrapper");
		}
	}
}