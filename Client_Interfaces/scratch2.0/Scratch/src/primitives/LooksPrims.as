/*
 * Scratch Project Editor and Player
 * Copyright (C) 2014 Massachusetts Institute of Technology
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

// LooksPrims.as
// John Maloney, April 2010
//
// Looks primitives.

package primitives {
	import flash.utils.Dictionary;
	import blocks.*;
	import interpreter.*;
	import scratch.*;

public class LooksPrims {

	private var app:Scratch;
	private var interp:Interpreter;

	public function LooksPrims(app:Scratch, interpreter:Interpreter) {
		this.app = app;
		this.interp = interpreter;
	}

	public function addPrimsTo(primTable:Dictionary):void {
		primTable['lookLike:']				= primShowCostume;
		primTable['nextCostume']			= primNextCostume;
		primTable['costumeIndex']			= primCostumeIndex;
		primTable['costumeName']			= primCostumeName;

		primTable['showBackground:']		= primShowCostume; // used by Scratch 1.4 and earlier (doesn't start scene hats)
		primTable['nextBackground']			= primNextCostume; // used by Scratch 1.4 and earlier (doesn't start scene hats)
		primTable['backgroundIndex']		= primSceneIndex;
		primTable['sceneName']				= primSceneName;
		primTable['nextScene']				= function(b:*):* { startScene('next backdrop', false) };
		primTable['startScene']				= function(b:*):* { startScene(interp.arg(b, 0), false) };
		primTable['startSceneAndWait']		= function(b:*):* { startScene(interp.arg(b, 0), true) };

		primTable['say:duration:elapsed:from:']		= function(b:*):* { showBubbleAndWait(b, 'talk') };
		primTable['say:']							= function(b:*):* { showBubble(b, 'talk') };
		primTable['think:duration:elapsed:from:']	= function(b:*):* { showBubbleAndWait(b, 'think') };
		primTable['think:']							= function(b:*):* { showBubble(b, 'think') };

		primTable['changeGraphicEffect:by:'] = primChangeEffect;
		primTable['setGraphicEffect:to:']	= primSetEffect;
		primTable['filterReset']			= primClearEffects;

		primTable['changeSizeBy:']			= primChangeSize;
		primTable['setSizeTo:']				= primSetSize;
		primTable['scale']					= primSize;

		primTable['show']					= primShow;
		primTable['hide']					= primHide;
//		primTable['hideAll']				= primHideAll;

		primTable['comeToFront']			= primGoFront;
		primTable['goBackByLayers:']		= primGoBack;

		primTable['setVideoState']			= primSetVideoState;
		primTable['setVideoTransparency']	= primSetVideoTransparency;

//		primTable['scrollAlign']			= primScrollAlign;
//		primTable['scrollRight']			= primScrollRight;
//		primTable['scrollUp']				= primScrollUp;
//		primTable['xScroll']				= function(b:*):* { return app.stagePane.xScroll };
//		primTable['yScroll']				= function(b:*):* { return app.stagePane.yScroll };

		primTable['setRotationStyle']		= primSetRotationStyle;
		
		// Muratet ---
		primTable['PP_Open']					= prim_PP_Open;
		primTable['PP_IsGameOver']				= prim_PP_IsGameOver;
		primTable['PP_IsGamePaused']			= prim_PP_IsGamePaused;
		primTable['PP_GetMapWidth']				= prim_PP_GetMapWidth;
		primTable['PP_GetMapHeight']			= prim_PP_GetMapHeight;
		primTable['PP_GetStartPosX']			= prim_PP_GetStartPosX;
		primTable['PP_GetStartPosY']			= prim_PP_GetStartPosY;
		primTable['PP_GetNumSpecialArea']		= prim_PP_GetNumSpecialArea;
		primTable['PP_GetSpecialAreaPosX']		= prim_PP_GetSpecialAreaPosX;
		primTable['PP_GetSpecialAreaPosY']		= prim_PP_GetSpecialAreaPosY;
		primTable['PP_GetResource']				= prim_PP_GetResource;
		primTable['PP_GetNumUnits']				= prim_PP_GetNumUnits;
		primTable['PP_GetUnitAt']				= prim_PP_GetUnitAt;
		primTable['PP_Unit_BelongTo']			= prim_PP_Unit_BelongTo;
		primTable['PP_Unit_IsType']				= prim_PP_Unit_IsType;
		primTable['PP_Unit_GetPositionX']		= prim_PP_Unit_GetPositionX;
		primTable['PP_Unit_GetPositionY']		= prim_PP_Unit_GetPositionY;
		primTable['PP_Unit_GetHealth']			= prim_PP_Unit_GetHealth;
		primTable['PP_Unit_GetMaxHealth']		= prim_PP_Unit_GetMaxHealth;
		primTable['PP_Unit_GetGroup']			= prim_PP_Unit_GetGroup;
		primTable['PP_Unit_SetGroup']			= prim_PP_Unit_SetGroup;
		primTable['PP_Unit_GetNumPendingCmds']	= prim_PP_Unit_GetNumPendingCmds;
		primTable['PP_Unit_PdgCmd_IsEqualTo']	= prim_PP_Unit_PdgCmd_IsEqualTo;
		primTable['PP_Unit_PdgCmd_GetNumParams']= prim_PP_Unit_PdgCmd_GetNumParams;
		primTable['PP_Unit_PdgCmd_GetParamAt']	= prim_PP_Unit_PdgCmd_GetParamAt;
		primTable['PP_Unit_ActionOnPosition']	= prim_PP_Unit_ActionOnPosition;
		primTable['PP_Unit_ActionOnUnit']		= prim_PP_Unit_ActionOnUnit;
		primTable['PP_Unit_ActionSetState']		= prim_PP_Unit_ActionSetState;
		primTable['PP_Unit_UntargetedAction']	= prim_PP_Unit_UntargetedAction;
		primTable['PP_Close']					= prim_PP_Close;
		primTable['PP_GetError']				= prim_PP_GetError;
		primTable['PP_ClearError']				= prim_PP_ClearError;
		// ---
	}

	// Muratet ---
	private var ppExt:ProgAndPlayANE = null;
	
	private function prim_PP_Open(b:Block):void {
 		if (!ppExt)
			ppExt = new ProgAndPlayANE();
 		if (ppExt)
			ppExt.PP_Open_ext();
		
	}
	
	private function prim_PP_Close(b:Block):void {
 		if (ppExt)
			ppExt.PP_Close_ext();
	}
	
	private function prim_PP_IsGameOver(b:Block):Boolean {
 		if (ppExt)
			return ppExt.PP_IsGameOver_ext();
		return false;
	}
	
	private function prim_PP_IsGamePaused(b:Block):Boolean {
 		if (ppExt)
			return ppExt.PP_IsGamePaused_ext();
		return false;
	}
	
	private function prim_PP_GetMapWidth(b:Block):int {
 		if (ppExt)
			return ppExt.PP_GetMapWidth_ext();
		return -1;
	}
	
	private function prim_PP_GetMapHeight(b:Block):int {
 		if (ppExt)
			return ppExt.PP_GetMapHeight_ext();
		return -1;
	}
	
	private function prim_PP_GetStartPosX(b:Block):Number {
 		if (ppExt)
			return ppExt.PP_GetStartPosX_ext();
		return -1;
	}
	
	private function prim_PP_GetStartPosY(b:Block):Number {
 		if (ppExt)
			return ppExt.PP_GetStartPosY_ext();
		return -1;
	}
	
	private function prim_PP_GetNumSpecialArea(b:Block):int {
 		if (ppExt)
			return ppExt.PP_GetNumSpecialArea_ext();
		return -1;
	}
	
	private function prim_PP_GetSpecialAreaPosX(b:Block):Number {
 		if (ppExt){
			var num:int = getInt(b, 0);
			return ppExt.PP_GetSpecialAreaPosX_ext(num);
		}
		return -1;
	}
	
	private function prim_PP_GetSpecialAreaPosY(b:Block):Number {
 		if (ppExt){
			var num:int = getInt(b, 0);
			return ppExt.PP_GetSpecialAreaPosY_ext(num);
		}
		return -1;
	}
	
	private function getCodeFromName (array:Array, name:String):int {
		var selected:Array = array.filter(function (element:*, index:int, arr:Array):Boolean {
			return (element.name == name);
		});
		if (selected.length > 0)
			return selected[0].code;
		else
			return -1;
	}
	
	private function prim_PP_GetResource(b:Block):int {
 		if (ppExt){
			var code:int = getCodeFromName(Specs.pp_resourcesList, interp.arg(b, 0) as String);
			if (code != -1)
				return ppExt.PP_GetResource_ext(code);
			else
				return -1;
		}
		return -1;
	}
	
	private function prim_PP_GetNumUnits(b:Block):int {
 		if (ppExt){
			var coalition:int = getCodeFromName(Specs.pp_coalitionsList, interp.arg(b, 0) as String);
			if (coalition != -1)
				return ppExt.PP_GetNumUnits_ext(coalition);
			else
				return -1;
		}
		return -1;
	}
	
	private function prim_PP_GetUnitAt(b:Block):int {
 		if (ppExt){
			var id:int = getInt(b, 0);
			var coalition:int = getCodeFromName(Specs.pp_coalitionsList, interp.arg(b, 1) as String);
			if (coalition != -1)
				return ppExt.PP_GetUnitAt_ext(id, coalition);
			else
				return -1;
		}
		return -1;
	}
	
	private function prim_PP_Unit_BelongTo(b:Block):Boolean {
 		if (ppExt){
			var unitId:int = getInt(b, 0);
			var coalition:int = getCodeFromName(Specs.pp_coalitionsList, interp.arg(b, 1) as String);
			if (coalition != -1)
				return ppExt.PP_Unit_BelongTo_ext(unitId, coalition);
			else
				return false;
		}
		return false;
	}
	
	private function prim_PP_Unit_IsType(b:Block):Boolean {
 		if (ppExt){
			var unitId:int = getInt(b, 0);
			var unitType:int = getCodeFromName(Specs.pp_unitsList, interp.arg(b, 1) as String);
			if (unitType != -1)
				return ppExt.PP_Unit_IsType_ext(unitId, unitType);
			else
				return false;
		}
		return false;
	}
	
	private function prim_PP_Unit_GetPositionX(b:Block):Number {
 		if (ppExt){
			var unitId:int = getInt(b, 0);
			return ppExt.PP_Unit_GetPositionX_ext(unitId);
		}
		return -1;
	}
	
	private function prim_PP_Unit_GetPositionY(b:Block):Number {
 		if (ppExt){
			var unitId:int = getInt(b, 0);
			return ppExt.PP_Unit_GetPositionY_ext(unitId);
		}
		return -1;
	}
	
	private function prim_PP_Unit_GetHealth(b:Block):Number {
 		if (ppExt){
			var unitId:int = getInt(b, 0);
			return ppExt.PP_Unit_GetHealth_ext(unitId);
		}
		return -1;
	}
	
	private function prim_PP_Unit_GetMaxHealth(b:Block):Number {
 		if (ppExt){
			var unitId:int = getInt(b, 0);
			return ppExt.PP_Unit_GetMaxHealth_ext(unitId);
		}
		return -1;
	}
	
	private function prim_PP_Unit_GetGroup(b:Block):int {
 		if (ppExt){
			var unitId:int = getInt(b, 0);
			return ppExt.PP_Unit_GetGroup_ext(unitId);
		}
		return -1;
	}
	
	private function prim_PP_Unit_SetGroup(b:Block):void {
 		if (ppExt){
			var unitId:int = getInt(b, 0);
			var groupId:int = getInt(b, 1);
			ppExt.PP_Unit_SetGroup_ext(unitId, groupId);
		}
	}
	
	private function prim_PP_Unit_GetNumPendingCmds(b:Block):int {
 		if (ppExt){
			var unitId:int = getInt(b, 0);
			return ppExt.PP_Unit_GetNumPendingCmds_ext(unitId);
		}
		return -1;
	}
	
	private function prim_PP_Unit_PdgCmd_IsEqualTo(b:Block):Boolean {
 		if (ppExt){
			var cmdId:int = getInt(b, 0);
			var unitId:int = getInt(b, 1);
			var cmdName:String = interp.arg(b, 2) as String;
			// try to find this command name on standard commands
			var cmdCode:int = getCodeFromName(Specs.pp_standardCommandsList, cmdName);
			if (cmdCode == -1)
				// we didn't find in standard commands, then we try to find this command name on specific commands
				cmdCode = getCodeFromName(Specs.pp_specificCommandsList, cmdName);
			if (cmdCode == -1)
				// we didn't find the code associated to this command
				return false
			else
				// we found a code, then we compare it with Prog&Play call result
				return ppExt.PP_Unit_PdgCmd_IsEqualTo_ext(cmdId, unitId, cmdCode);
		}
		return false;
	}
	
	private function prim_PP_Unit_PdgCmd_GetNumParams(b:Block):int {
 		if (ppExt){
			var cmdId:int = getInt(b, 0);
			var unitId:int = getInt(b, 1);
			return ppExt.PP_Unit_PdgCmd_GetNumParams_ext(cmdId, unitId);
		}
		return -1;
	}
	
	private function prim_PP_Unit_PdgCmd_GetParamAt(b:Block):Number {
 		if (ppExt){
			var paramId:int = getInt(b, 0);
			var cmdId:int = getInt(b, 1);
			var unitId:int = getInt(b, 2);
			return ppExt.PP_Unit_PdgCmd_GetParamAt_ext(paramId, cmdId, unitId);
		}
		return -1;
	}
	
	private function prim_PP_Unit_ActionOnPosition(b:Block):void {
 		if (ppExt){
			var unitId:int = getInt(b, 0);
			var cmdName:String = interp.arg(b, 1) as String;
			var cmdCode:int = getCodeFromName(Specs.pp_standardCommandsList, cmdName);
			if (cmdCode == -1)
				// we didn't find in standard commands, then we try to find this command name on specific commands
				cmdCode = getCodeFromName(Specs.pp_specificCommandsList, cmdName);
			if (cmdCode != -1){
				var x:Number = getNumber(b, 2);
				var y:Number = getNumber(b, 3);
				var synchroName:String = interp.arg(b, 4) as String;
				var synchroCode:int = getCodeFromName(Specs.pp_synchroList, synchroName);
				if (synchroCode == -1)
					synchroCode = 0;
				ppExt.PP_Unit_ActionOnPosition_ext(unitId, cmdCode, x, y, synchroCode);
			}
		}
	}
	
	private function prim_PP_Unit_ActionOnUnit(b:Block):void {
 		if (ppExt){
			var unitId:int = getInt(b, 0);
			var cmdName:String = interp.arg(b, 1) as String;
			var cmdCode:int = getCodeFromName(Specs.pp_standardCommandsList, cmdName);
			if (cmdCode == -1)
				// we didn't find in standard commands, then we try to find this command name on specific commands
				cmdCode = getCodeFromName(Specs.pp_specificCommandsList, cmdName);
			if (cmdCode != -1){
				var targetId:int = getInt(b, 2);
				var synchroName:String = interp.arg(b, 3) as String;
				var synchroCode:int = getCodeFromName(Specs.pp_synchroList, synchroName);
				if (synchroCode == -1)
					synchroCode = 0;
				ppExt.PP_Unit_ActionOnUnit_ext(unitId, cmdCode, targetId, synchroCode);
			}
		}
	}
	
	private function prim_PP_Unit_ActionSetState(b:Block):void {
 		if (ppExt){
			var unitId:int = getInt(b, 0);
			var cmdName:String = interp.arg(b, 1) as String;
			var cmdCode:int = getCodeFromName(Specs.pp_standardCommandsList, cmdName);
			if (cmdCode == -1)
				// we didn't find in standard commands, then we try to find this command name on specific commands
				cmdCode = getCodeFromName(Specs.pp_specificCommandsList, cmdName);
			if (cmdCode != -1){
				var param:Number = getNumber(b, 2);
				var synchroName:String = interp.arg(b, 3) as String;
				var synchroCode:int = getCodeFromName(Specs.pp_synchroList, synchroName);
				if (synchroCode == -1)
					synchroCode = 0;
				ppExt.PP_Unit_UntargetedAction_ext(unitId, cmdCode, param, synchroCode);
			}
		}
	}
	
	private function prim_PP_Unit_UntargetedAction(b:Block):void {
 		if (ppExt){
			var unitId:int = getInt(b, 0);
			var cmdName:String = interp.arg(b, 1) as String;
			var cmdCode:int = getCodeFromName(Specs.pp_standardCommandsList, cmdName);
			if (cmdCode == -1)
				// we didn't find in standard commands, then we try to find this command name on specific commands
				cmdCode = getCodeFromName(Specs.pp_specificCommandsList, cmdName);
			if (cmdCode != -1){
				var synchroName:String = interp.arg(b, 2) as String;
				var synchroCode:int = getCodeFromName(Specs.pp_synchroList, synchroName);
				if (synchroCode == -1)
					synchroCode = 0;
				ppExt.PP_Unit_UntargetedActionWithoutParam_ext(unitId, cmdCode, synchroCode);
			}
		}
	}
	
	private function prim_PP_GetError(b:Block):String {
 		if (ppExt)
			return ppExt.PP_GetError_ext();
		else
			return "Call \"open Prog&Play\" block first";
	}
	
	private function prim_PP_ClearError(b:Block):void {
 		if (ppExt)
			ppExt.PP_ClearError_ext();
	}
	
	private function getInt(b:Block, id:int):int {
		var n:* = interp.arg(b, id);
		trace(n);
		var i:int;
		if (!(n is Number)) {
			n = Number(n);
		trace(n);
			if (isNaN(n)) return -1;
		}
		i = (n is int) ? n : Math.floor(n);
		trace(i);
		return i;
	}
	
	private function getNumber(b:Block, id:int):Number {
		var n:* = interp.arg(b, id);
		if (!(n is Number)) {
			n = Number(n);
			if (isNaN(n)) return -1;
		}
		return n;
	}
		
	// ---
	
	private function primNextCostume(b:Block):void {
		var s:ScratchObj = interp.targetObj();
		if (s != null) s.showCostume(s.currentCostumeIndex + 1);
		if (s.visible) interp.redraw();
	}

	private function primShowCostume(b:Block):void {
		var s:ScratchObj = interp.targetObj();
		if (s == null) return;
		var arg:* = interp.arg(b, 0);
		if (typeof(arg) == 'number') {
			s.showCostume(arg - 1);
		} else {
			var i:int = s.indexOfCostumeNamed(arg);
			if (i >= 0) {
				s.showCostume(i);
			} else if ('previous costume' == arg) {
				s.showCostume(s.currentCostumeIndex - 1);
			} else if ('next costume' == arg) {
				s.showCostume(s.currentCostumeIndex + 1);
			} else {
				var n:Number = Interpreter.asNumber(arg);
				if (!isNaN(n)) s.showCostume(n - 1);
				else return; // arg did not match a costume name nor is it a valid number
			}
		}
		if (s.visible) interp.redraw();
	}

	private function primCostumeIndex(b:Block):Number {
		var s:ScratchObj = interp.targetObj();
		return (s == null) ? 1 : s.costumeNumber();
	}

	private function primCostumeName(b:Block):String {
		var s:ScratchObj = interp.targetObj();
		return (s == null) ? '' : s.currentCostume().costumeName;
	}

	private function primSceneIndex(b:Block):Number {
		return app.stagePane.costumeNumber();
	}

	private function primSceneName(b:Block):String {
		return app.stagePane.currentCostume().costumeName;
	}

	private function startScene(s:String, waitFlag:Boolean):void {
		if ('next backdrop' == s) s = backdropNameAt(app.stagePane.currentCostumeIndex + 1);
		else if ('previous backdrop' == s) s = backdropNameAt(app.stagePane.currentCostumeIndex - 1);
		else {
			var n:Number = Interpreter.asNumber(s);
			if (!isNaN(n)) {
				n = (Math.round(n) - 1) % app.stagePane.costumes.length;
				if (n < 0) n += app.stagePane.costumes.length;
				s = app.stagePane.costumes[n].costumeName;
			}
		}
		interp.startScene(s, waitFlag);
	}

	private function backdropNameAt(i:int):String {
		var costumes:Array = app.stagePane.costumes;
		return costumes[(i + costumes.length) % costumes.length].costumeName;
	}

	private function showBubbleAndWait(b:Block, type:String):void {
		var text:*, secs:Number;
		var s:ScratchSprite = interp.targetSprite();
		if (s == null) return;
		if (interp.activeThread.firstTime) {
			text = interp.arg(b, 0);
			secs = interp.numarg(b, 1);
			s.showBubble(text, type, b);
			if (s.visible) interp.redraw();
			interp.startTimer(secs);
		} else {
			if (interp.checkTimer() && s.bubble && (s.bubble.getSource() == b)) {
				s.hideBubble();
			}
		}
	}

	private function showBubble(b:Block, type:String = null):void {
		var text:*, secs:Number;
		var s:ScratchSprite = interp.targetSprite();
		if (s == null) return;
		if (type == null) { // combined talk/think/shout/whisper command
			type = interp.arg(b, 0);
			text = interp.arg(b, 1);
		} else { // talk or think command
			text = interp.arg(b, 0);
		}
		s.showBubble(text, type, b);
		if (s.visible) interp.redraw();
	}

	private function primChangeEffect(b:Block):void {
		var s:ScratchObj = interp.targetObj();
		if (s == null) return;
		var filterName:String = interp.arg(b, 0);
		var delta:Number = interp.numarg(b, 1);
		if(delta == 0) return;

		var newValue:Number = s.filterPack.getFilterSetting(filterName) + delta;
		s.filterPack.setFilter(filterName, newValue);
		s.applyFilters();
		if (s.visible || s == Scratch.app.stagePane) interp.redraw();
	}

	private function primSetEffect(b:Block):void {
		var s:ScratchObj = interp.targetObj();
		if (s == null) return;
		var filterName:String = interp.arg(b, 0);
		var newValue:Number = interp.numarg(b, 1);
		if(s.filterPack.setFilter(filterName, newValue))
			s.applyFilters();
		if (s.visible || s == Scratch.app.stagePane) interp.redraw();
	}

	private function primClearEffects(b:Block):void {
		var s:ScratchObj = interp.targetObj();
		s.clearFilters();
		s.applyFilters();
		if (s.visible || s == Scratch.app.stagePane) interp.redraw();
	}

	private function primChangeSize(b:Block):void {
		var s:ScratchSprite = interp.targetSprite();
		if (s == null) return;
		var oldScale:Number = s.scaleX;
		s.setSize(s.getSize() + interp.numarg(b, 0));
		if (s.visible && (s.scaleX != oldScale)) interp.redraw();
	}

	private function primSetRotationStyle(b:Block):void {
		var s:ScratchSprite = interp.targetSprite();
		var newStyle:String = interp.arg(b, 0) as String;
		if ((s == null) || (newStyle == null)) return;
		s.setRotationStyle(newStyle);
	}

	private function primSetSize(b:Block):void {
		var s:ScratchSprite = interp.targetSprite();
		if (s == null) return;
		s.setSize(interp.numarg(b, 0));
		if (s.visible) interp.redraw();
	}

	private function primSize(b:Block):Number {
		var s:ScratchSprite = interp.targetSprite();
		if (s == null) return 100;
		return Math.round(s.getSize()); // reporter returns rounded size, as in Scratch 1.4
	}

	private function primShow(b:Block):void {
		var s:ScratchSprite = interp.targetSprite();
		if (s == null) return;
		s.visible = true;
		if(!app.isIn3D) s.applyFilters();
		s.updateBubble();
		if (s.visible) interp.redraw();
	}

	private function primHide(b:Block):void {
		var s:ScratchSprite = interp.targetSprite();
		if ((s == null) || !s.visible) return;
		s.visible = false;
		if(!app.isIn3D) s.applyFilters();
		s.updateBubble();
		interp.redraw();
	}

	private function primHideAll(b:Block):void {
		// Hide all sprites and delete all clones. Only works from the stage.
		if (!interp.targetObj().isStage) return;
		app.stagePane.deleteClones();
		for (var i:int = 0; i < app.stagePane.numChildren; i++) {
			var o:* = app.stagePane.getChildAt(i);
			if (o is ScratchSprite) {
				o.visible = false;
				o.updateBubble();
			}
		}
		interp.redraw();
	}

	private function primGoFront(b:Block):void {
		var s:ScratchSprite = interp.targetSprite();
		if ((s == null) || (s.parent == null)) return;
		s.parent.setChildIndex(s, s.parent.numChildren - 1);
		if (s.visible) interp.redraw();
	}

	private function primGoBack(b:Block):void {
		var s:ScratchSprite = interp.targetSprite();
		if ((s == null) || (s.parent == null)) return;
		var newIndex:int = s.parent.getChildIndex(s) - interp.numarg(b, 0);
		newIndex = Math.max(minSpriteLayer(), Math.min(newIndex, s.parent.numChildren - 1));

		if (newIndex > 0 && newIndex < s.parent.numChildren) {
			s.parent.setChildIndex(s, newIndex);
			if (s.visible) interp.redraw();
		}
	}

	private function minSpriteLayer():int {
		// Return the lowest sprite layer.
		var stg:ScratchStage = app.stagePane;
		return stg.getChildIndex(stg.videoImage ? stg.videoImage : stg.penLayer) + 1;
	}

	private function primSetVideoState(b:Block):void {
		app.stagePane.setVideoState(interp.arg(b, 0));
	}

	private function primSetVideoTransparency(b:Block):void {
		app.stagePane.setVideoTransparency(interp.numarg(b, 0));
		app.stagePane.setVideoState('on');
	}

	private function primScrollAlign(b:Block):void {
		if (!interp.targetObj().isStage) return;
		app.stagePane.scrollAlign(interp.arg(b, 0));
	}

	private function primScrollRight(b:Block):void {
		if (!interp.targetObj().isStage) return;
		app.stagePane.scrollRight(interp.numarg(b, 0));
	}

	private function primScrollUp(b:Block):void {
		if (!interp.targetObj().isStage) return;
		app.stagePane.scrollUp(interp.numarg(b, 0));
	}
}}
