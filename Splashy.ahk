Class Splashy
{

	spr := 0
	spr1 := 0
	spr2 := 0

	; HTML Colours (RGB- no particular order)
	STATIC HTML := {CYAN: "0X00FFFF", AQUAMARINE : "0X7FFFD4", BLACK: "0X000000", BLUE: "0X0000FF", FUCHSIA: "0XFF00FF", GRAY: "0X808080", AUBURN: "0X2A2AA5"
	 , LIME: "0X00FF00", MAROON: "0X800000", NAVY: "0X000080", OLIVE: "0X808000", PURPLE: "0X800080", INDIGO: "0X4B0082", LAVENDER: "0XE6E6FA", DKSALMON: "0X7A96E9"
	 , SILVER: "0XC0C0C0", TEAL: "0X008080", WHITE: "0XFFFFFF", YELLOW: "0XFFFF00", WHEAT: "0xF5DEB3", ORANGE: "0XFFA500", BEIGE: "0XF5F5DC", CELADON: "0XACE1AF"
	 , CHESTNUT: "0X954535", TAN: "0xD2B48C", CHOCOLATE: "0X7B3F00", TAUPE: "0X483C32", SALMON: "0XFA8072", VIOLET: "0X7F00FF", GRAPE: "0X6F2DA8", STEINGRAU: "0X485555"
	 , PEACH: "0XFFE5B4", CORAL: "0XFF7F50", CRIMSON: "0XDC143C", VERMILION: "0XE34234", CERULEAN: "0X007BA7", TURQUOISE: "0X40E0D0", VIRIDIAN: "0X40826D", RED: "0XFF0000"
	 , PLUM: "0X8E4585", MAGENTA: "0XF653A6", GOLD: "0XFFD700", GOLDENROD: "0XDAA520", GREEN: "0X008000", ONYX: "0X353839", KHAKIGRAU: "0X746643", FELDGRAU: "0X3D5D5D"}

	Static MaxGuis := 1000	; arbitrary limit
	Static parentHWnd := 0
	Static updateFlag := -1
	Static procEnd := 0
	Static pToken := 0
	Static hGDIPLUS := 0

	Static parentClip := 0
	Static downloadedPathNames := []
	Static downloadedUrlNames := []
	Static NewWndObj := {}
	Static vImgType := 0
	Static hWndSaved := []
	Static parent := 0
	Static release := 0
	Static hDCWin := 0
	Static instance := 1
	Static oldInstance := 1
	Static hBitmap := 0
	Static hIcon := 0
	Static vImgTxtSize := 0
	Static vPosX := "c"
	Static vPosY := "c"
	Static vMgnX := 0
	Static vMgnY := 0
	Static vImgX := 0
	Static vImgY := 0
	Static inputVImgW := ""
	Static inputVImgH := ""
	Static vImgW := 0
	Static vImgH := 0
	Static oldVImgW := 0
	Static oldVImgH := 0
	Static actualVImgW := 0
	Static actualVImgH := 0
	Static oldPicInScript := 0
	Static picInScript := 0

	Static ImageName := ""
	Static oldImagePath := ""
	Static imageUrl := ""
	Static oldImageUrl := ""
	Static bkgdColour := ""
	Static transCol := 0
	Static vHide := 0
	Static noHWndActivate := ""
	Static vBorder := 0
	Static voldBorder := 0
	Static vOnTop := 0


	Static mainTextHWnd := []
	Static mainText := ""
	Static mainBkgdColour := ""
	Static mainFontName := ""
	Static mainFontSize := 0
	Static mainFontWeight := 0
	Static mainFontColour := ""
	Static mainFontQuality := 0
	Static mainFontItalic := ""
	Static mainFontStrike := ""
	Static mainFontUnderline := ""
	Static mainMarquee := 0

	Static subTextHWnd := []
	Static subText := ""
	Static subBkgdColour := ""
	Static subFontName := ""
	Static subFontSize := 0
	Static subFontWeight := 0
	Static subFontColour := ""
	Static subFontQuality := 0
	Static subFontItalic := ""
	Static subFontStrike := ""
	Static subFontUnderline := ""
	Static subMarquee := 0

	Class NewWndProc
	{
	Static clbk := []
	Static wndProcOld := 0
	Static WM_PAINT := 0x000F
	Static WM_NCHITTEST := 0x84
	Static WM_ERASEBKGND := 0x0014
	Static WM_CTLCOLORSTATIC := 0x0138
	Static WM_ENTERSIZEMOVE := 0x0231

		__New(instance)
		{
		Static SetWindowLong := A_PtrSize == 8 ? "SetWindowLongPtr" : "SetWindowLong"
		Static wndProcNew := 0

			if (!(wndProcNew := This.clbk[instance].addr)) ; called once only from the caller- this is extra security
			{
			This.clbk[instance] := new This.BoundFuncCallback( ObjBindMethod(This, "WndProc"), 4 )
			wndProcNew := This.clbk[instance].addr
				if (wndProcNew)
				{
					if (!(This.wndProcOld := DllCall(SetWindowLong, "Ptr", Splashy.hWnd(), "Int", GWL_WNDPROC := -4, "Ptr", wndProcNew, "Ptr")))
					msgbox, 8208, WndProc, Bad return!		
				}
				else
				msgbox, 8208, WndProc, No address!
			}
		}

		__Delete(instance)
		{
			This.clbk[instance] := "" ; triggers clbk.__Delete()
		}

		class BoundFuncCallback
		{
		; https://www.autohotkey.com/boards/viewtopic.php?f=76&t=88704&p=390706
			__New(BoundFuncObj, paramCount, options := "")
			{
			This.pInfo := Object( {BoundObj: BoundFuncObj, paramCount: paramCount} )
			This.addr := RegisterCallback(This.__Class . "._Callback", options, paramCount, This.pInfo)
			}
			__Delete()
			{
			ObjRelease(This.pInfo)
			if (DllCall("GlobalFree", "Ptr", This.addr, "Ptr"))
			msgbox, 8208, GlobalFree, Memory could not be released!
			}
			_Callback(Params*)
			{
			Info := Object(A_EventInfo), Args := []
			Loop % Info.paramCount
			Args.Push( NumGet(Params + A_PtrSize*(A_Index - 2)) )
			Return Info.BoundObj.Call(Args*)
			}
		}

		WndProc(hwnd, uMsg, wParam, lParam)
		{
		;Critical 
			Switch uMsg
			{
				case % This.WM_CTLCOLORSTATIC:
				{
				return This.CtlColorStaticProc(wParam, lParam)
				}
				case % This.WM_PAINT:
				{
				This.PaintProc()
				return 0
				}
				case % This.WM_ENTERSIZEMOVE:
				{
					; For WM_Move: revert the parent for the window move
					if (Splashy.parent)
					{
						if (StrLen(Splashy.mainText))
						Splashy.SetParent(0, 0)
						if (StrLen(Splashy.subText))
						Splashy.SetParent(0, , 0)
					}
				return 0
				}
				case % This.WM_NCHITTEST:
				{
				return This.NcHitTestProc(hWnd, uMsg, wParam, lParam)
				}
				Default:
				return DllCall("CallWindowProc", "Ptr", This.wndProcOld, "Ptr", hWnd, "UInt", uMsg, "Ptr", wParam, "Ptr", lParam)
			}
			
		}

		NcHitTestProc(hWnd, uMsg, wParam, lParam)
		{
			Static HTCLIENT := 1, HTCAPTION := 2
			; Makes form movable

			if (Splashy.vMovable)
			{

			lResult := DllCall("DefWindowProc", "Ptr", hWnd, "UInt", uMsg, "UPtr", wParam, "Ptr", lParam)

				if (lResult == HTCLIENT)
				lResult := HTCAPTION
			return % lResult
			}
			else
			return % HTCLIENT
		}

		CtlColorStaticProc(wParam, lParam)
		{
		static DC_BRUSH := 0x12
 
			if (lparam == Splashy.subTextHWnd[Splashy.instance]) ; && This.hDCWin == wParam)
			This.SetColour(wParam, Splashy.subBkgdColour, Splashy.subFontColour)
			else
			{
				if (lParam == Splashy.mainTextHWnd[Splashy.instance])
				This.SetColour(wParam, Splashy.mainBkgdColour, Splashy.mainFontColour)
			}
		return DllCall("Gdi32.dll\GetStockObject", "UInt", DC_BRUSH, "UPtr")
		}

		SetColour(textDC, bkgdColour, fontColour)
		{
			static NULL_BRUSH := 0x5, TRANSPARENT := 0X1, OPAQUE := 0X2, CLR_INVALID := 0xFFFFFFFF

			DllCall("Gdi32.dll\SetBkMode", "Ptr", textDC, "UInt", (Splashy.transCol)? TRANSPARENT: OPAQUE)
			if (DllCall("Gdi32.dll\SetBkColor", "Ptr", textDC, "UInt", bkgdColour) == CLR_INVALID)
			msgbox, 8208, SetBkColor, Cannot set background colour for text!
			if (DllCall("Gdi32.dll\SetTextColor", "Ptr", textDC, "UInt", fontColour) == CLR_INVALID)
			msgbox, 8208, SetTextColor, Cannot set colour for text!

			if (DllCall("SetDCBrushColor", "Ptr", textDC, "UInt", bkgdColour) == CLR_INVALID)
			msgbox, 8208, SetDCBrushColor, Cannot set colour for brush!
		}


		PaintProc(hWnd := 0)
		{
		spr := 0	
			if (VarSetCapacity(PAINTSTRUCT, A_PtrSize + A_PtrSize + 56, 0)) ; hdc, rcPaint are pointers
			{
					if (!hWnd)
					{
					hWnd := Splashy.hWnd()
						if (!Splashy.procEnd)
						spr := 1
					}
				; DC validated
					if (DllCall("User32.dll\BeginPaint", "Ptr", hWnd, "Ptr", &PAINTSTRUCT, "UPtr"))
					{
						if (!spr)
						{
						static vDoDrawImg := 1 ;set This to 0 and the image won't be redrawn
						static vDoDrawBgd := 1 ;set This to 0 and the background won't be redrawn
						;return ;uncomment This line and the window will be blank

							if (vDoDrawImg)
							Splashy.PaintDC()

							if (vDoDrawBgd)
							Splashy.DrawBackground()
						}
					}
				DllCall("User32.dll\EndPaint", "Ptr", hWnd, "Ptr", &PAINTSTRUCT, "UPtr")
			}
			else
			msgbox, 8208, PAINTSTRUCT, Cannot paint!
		}

		SubClassTextCtl(ctlHWnd, release := 0)
		{
		Static SubProcFunc := 0
			if (release)
			{
			This.subClbk := "" ; triggers subClbk.__Delete()
			SubProcFunc := 0
			}
			else
			{
				if (!ctlHWnd)
				return
				if (!SubProcFunc)
				{
				This.subClbk := new This.BoundFuncCallback(ObjBindMethod(This, "SubClassTextProc"), 6)
				SubProcFunc := This.subClbk.addr
				}

				if !DllCall("Comctl32.dll\SetWindowSubclass", "Ptr", ctlHWnd, "Ptr", SubProcFunc, "Ptr", ctlHWnd, "Ptr", 0)
				msgbox, 8208, Text Control, SubClassing failed!

			}
		}
		SubClassTextProc(hWnd, uMsg, wParam, lParam, IdSubclass, RefData)
		{
		;THis subclass for marquee code
		; WM_PAINT is required to paint the scrolled text.
		; BeginPaint in WM_PAINT will erase the content already set in the DC of the control.
		;
		;To prevent, temporarily modify AHK's own hbrBackground in its WNDCLASSEX
		; hbrBackground := DllCall(GetStockObject(NULL_BRUSH))
		;
		;Which means we have to create our own class and window for the control anyway,
		; Then use Pens & Brushes & DrawTextEx et al.

		Critical
		static DC_BRUSH := 0x12
		/*



			if (uMsg == This.WM_ERASEBKGND)
			{
			return 1
			}
			if (uMsg == This.WM_PAINT)
			{
			VarSetCapacity(PAINTSTRUCT, A_PtrSize + A_PtrSize + 56, 0)
				if (!(hDC := DllCall("User32.dll\BeginPaint", "Ptr", hWnd, "Ptr", &PAINTSTRUCT, "UPtr")))
				return 0
			spr := This.ToBase(hWnd, 16)
			spr1 := InStr(spr, This.mainTextHWnd[This.instance])
			spr2 := InStr(spr, This.subTextHWnd[This.instance])


				if (spr1 || spr2)
				{

				if (This.mainMarquee && spr1)
				{
				}
				else
					{
						if (This.mainMarquee && spr1)
						{
						}
						else
						{
							if (spr1)
							This.SetColour(This.mainBkgdColour, hDC)
							else
							This.SetColour(This.subBkgdColour, hDC)
						}
					}
				}

			DllCall("User32.dll\EndPaint", "Ptr", hWnd, "Ptr", &PAINTSTRUCT, "UPtr")
			return 0
			}

	;Marquee code in an outside function
				;RECT rectControls := {wd + xCurrentScroll, yCurrentScroll, xNewSize + xCurrentScroll, yNewSize + yCurrentScroll};
				;if (!ScrollDC(hdcWinCl, -xDelta, 0, (CONST RECT*) &rectControls, (CONST RECT*) &rectControls, (HRGN)NULL, (RECT*) &rectControls))
					;ReportErr(L"HORZ_SCROLL: ScrollD Failed!");

	*/

		Return DllCall("Comctl32.dll\DefSubclassProc", "Ptr", hWnd, "UInt", uMsg, "Ptr", wParam, "Ptr", lParam)
		}

	}


	SplashImg(argList*)
	{
	parentOut := ""
	imagePathOut := ""
	imageUrlOut := ""
	bkgdColourOut := -1
	transColOut := ""
	vHideOut := 0
	noHWndActivateOut := ""
	vImgTxtSizeOut := 0
	vMovableOut := 0
	vBorderOut := ""
	vOnTopOut := 0
	vPosXOut := ""
	vPosYOut := ""
	vMgnXOut := 0
	vMgnYOut := 0
	vImgWOut := 0
	vImgHOut := 0
	mainTextOut := ""
	mainBkgdColourOut := -1
	mainFontNameOut := ""
	mainFontSizeOut := 0
	mainFontWeightOut := 0
	mainFontColourOut := -1
	mainFontQualityOut := -1
	mainFontItalicOut := 0
	mainFontStrikeOut := 0
	mainFontUnderlineOut := 0
	subTextOut := ""
	subBkgdColourOut := -1
	subFontNameOut := ""
	subFontSizeOut := 0
	subFontWeightOut := 0
	subFontColourOut := -1
	subFontQualityOut := -1
	subFontItalicOut := 0
	subFontStrikeOut := 0
	subFontUnderlineOut := 0

	This.SaveRestoreUserParms()

	StringCaseSense, Off
	SetWorkingDir %A_ScriptDir%

		if (argList["release"])
		{
			This.Destroy()
			return
		}

		if (argList.HasKey("instance"))
		{
			key := argList["instance"]
			if ((key := Floor(key))) ; 0 is invalid
			{
				if (This.hWndSaved[key])
				{
				This.instance := key

					if (This.instance != This.oldInstance)
					{
					; Ensures current postion is not reset
						This.vPosX := ""
						This.vPosY := ""
					}
				}
				else
				{
					if (key < 0)
					{
					key := -key
						if (This.hWndSaved[key])
						{
						;WinClose, % "ahk_id " This.hWndSaved[This.instance]
						This.hWndSaved[key] := 0
						This.mainTextHWnd[key] := 0
						This.subTextHWnd[key] := 0
						spr := "Splashy" . key
						Gui, %spr%: Destroy
						Splashy.NewWndProc.clbk[key] := ""

						;Now reset This.instance for next call
							if (This.hWndSaved.Length() == 1)
							This.instance := 1
							else
							{
								if (This.hWndSaved.Length() == key)
								This.instance -= 1
								else
								This.instance := This.hWndSaved.MaxIndex()
							}
						This.oldInstance := This.instance
						}
					This.SaveRestoreUserParms(1)
					return
					}
					else
					{
						if (key > This.MaxGuis)
						{
						This.SaveRestoreUserParms(1)
						return
						}
						else
						This.instance := key
					}
				}
			}
			else
			{
			This.SaveRestoreUserParms(1)
			return
			}
		}

		if (This.hWndSaved[This.instance])
		{
			if (argList.HasKey("initSplash"))
			{
				if (argList["initSplash"])
				This.updateFlag := 0
				else
				This.updateFlag := 1
			}
			else
			This.updateFlag := 1
		}
		else
		{
		; init the parent hWnd
			if (!This.parentHWnd)
			{
				This.parentHWnd := This.SetParentFlag()
				if (This.parentHWnd == "Error")
				{
				msgbox, 8192, Parent Script, Warning: Parent script is not AHK, or the window handle cannot be obtained!
				This.parentHWnd := 0
				}
			}
		}


		For key, value in argList
		{

		; An alternative: https://www.autohotkey.com/boards/viewtopic.php?f=6&t=9656
			if (key)
			{
			; Validate values
				Switch key
				{

				Case "parent":
				{
					if (This.updateFlag > 0)
					This.parent := value
					else
					parentOut := value
				}

				Case "imagePath":
				{
					if (This.updateFlag > 0)
					This.imagePath := This.ValidateText(value)
					else
					imagePathOut := value

					if ((InStr(This.imagePath, "*")))
					This.picInScript := 1
					else
					This.picInScript := 0
				}

				Case "imageUrl":
				{
				value := Trim(Value)

					if (This.updateFlag > 0)
					This.imageUrl := This.ValidateText(value)
					else
					imageUrlOut := value
				}
				Case "bkgdColour":
				{
					if (value != -1)
					{
						if (This.updateFlag > 0)
						This.bkgdColour := This.ValidateColour(value)
						else
						bkgdColourOut := value
					}
				}
				Case "transCol":
				{
					if (This.transCol)
					{
						if (!value && This.hWndSaved[This.instance])
						{
						WinSet, TransColor, Off, % "ahk_id" . This.hWndSaved[This.instance]
							if (This.subTextHWnd[This.instance])
							WinSet, TransColor, Off, % "ahk_id" . This.subTextHWnd[This.instance]
							if (This.mainTextHWnd[This.instance])
							WinSet, TransColor, Off, % "ahk_id" . This.mainTextHWnd[This.instance]
						}
					}

					if (This.updateFlag > 0)
					This.transCol := value
					else
					transColOut := value
				}
				Case "vHide":
				{
					if (This.updateFlag > 0)
					This.vHide := value
					else
					vHideOut := value
				}
				Case "noHWndActivate":
				{
					if (This.updateFlag > 0)
					This.noHWndActivate := (value)? "NoActivate ": ""
					else
					noHWndActivateOut := value
				}
				Case "vOnTop":
				{
					if (This.updateFlag > 0)
					This.vOnTop := value
					else
					vOnTopOut := value
				}
				Case "vMovable":
				{
					if (This.updateFlag > 0)
					This.vMovable := value
					else
					vMovableOut := value
				}
				Case "vBorder":
				{
					if (InStr(value, "b"))
					spr := "b"
					else
					{
					spr := ""
						if (InStr(value, "w"))
						spr .= "w"
						if (InStr(value, "s"))
						spr .= "s"
						if (InStr(value, "c"))
						spr .= "c"
						if (InStr(value, "d"))
						spr .= "d"
						if (value && !spr)
						spr := "dlgframe"
					}
					if (This.updateFlag > 0)
					This.vBorder := spr
					else
					vBorderOut := spr
				}
				Case "vImgTxtSize":
				{
					if (This.updateFlag > 0)
					This.vImgTxtSize := value
					else
					vImgTxtSizeOut := value
				}
				Case "vPosX":
				{
					if value is number
					{
						if (value)
						spr := Floor(value)
						else
						spr := "zero"
					}
					else
					spr := (Instr(value, "c"))? "c": ""

					if (This.updateFlag > 0)
					This.vPosX := spr
					else
					vPosXOut := spr
				}

				Case "vPosY":
				{
					if value is number
					{
						if (value)
						spr := Floor(value)
						else
						spr := "zero"
					}
					else
					spr := (Instr(value, "c"))? "c": ""

					if (This.updateFlag > 0)
					This.vPosY := spr
					else
					vPosYOut := spr
				}

				Case "vMgnX":
				{
					if (value >= 0)
					{
					spr := Instr(value, "d")? "d": Floor(value)
						if (This.updateFlag > 0)
						This.vMgnX := spr
						else
						vMgnXOut := spr
					}
				}
				Case "vMgnY":
				{
					if (value >= 0)
					{
					spr := Instr(value, "d")? "d": Floor(value)
						if (This.updateFlag > 0)
						This.vMgnY := spr
						else
						vMgnYOut := spr
					}
				}

				Case "vImgW":
				{
				spr := (This.actualVImgW)?This.ProcImgWHVal(value):value

					if (This.updateFlag > 0)
					This.inputVImgW := spr
					else
					vImgWOut := spr
				}

				Case "vImgH":
				{
				spr := (This.actualVImgH)?This.ProcImgWHVal(value, 1):value

					if (This.updateFlag > 0)
					This.inputVImgH := spr
					else
					vImgHOut := spr
				}


				Case "mainText":
				{
					if (This.updateFlag > 0)
					This.mainText := This.ValidateText(value)
					else
					mainTextOut := value
				}
				Case "mainBkgdColour":
				{
					if (value != -1)
					{
						if (This.updateFlag > 0)
						This.mainBkgdColour := This.ValidateColour(value, 1)
						else
						mainBkgdColourOut := value
					}
				}
				Case "mainFontName":
				{
					if (value)
					{
						if (This.updateFlag > 0)
						This.mainFontName := This.ValidateText(value)
						else
						mainFontNameOut := value
					}
				}
				Case "mainFontSize":
				{
					if (200 >= value >= 0) ; arbitrary limit
					{
						if (This.updateFlag > 0)
						This.mainFontSize := Floor(value)
						else
						mainFontSizeOut := value
					}
				}
				Case "mainFontWeight":
				{
					if (1000 >= value >= 0)
					{
						if (This.updateFlag > 0)
						This.mainFontWeight := Floor(value)
						else
						mainFontWeightOut := value
					}
				}
				Case "mainFontColour":
				{
					if (value != -1)
					{
						if (This.updateFlag > 0)
						This.mainFontColour := This.ValidateColour(value, 1)
						else
						mainFontColourOut := value
					}
				}
				Case "mainFontQuality":
				{
					if (value >= 0 && value <= 5) ; 0 :=  DEFAULT_QUALITY
					{
						if (This.updateFlag > 0)
						This.mainFontQuality := Floor(value)
						else
						mainFontQualityOut := value
					}
				}
				Case "mainFontItalic":
				{
					if (This.updateFlag > 0)
					This.mainFontItalic := (value)? " Italic": ""
					else
					mainFontItalicOut := value
				}
				Case "mainFontStrike":
				{
					if (This.updateFlag > 0)
					This.mainFontStrike := (value)? " Strike": ""
					else
					mainFontStrikeOut := value
				}
				Case "mainFontUnderline":
				{
					if (This.updateFlag > 0)
					This.mainFontUnderline := (value)? " Underline": ""
					else
					mainFontUnderlineOut := value
				}




				Case "subText":
				{
					if (This.updateFlag > 0)
					This.subText := This.ValidateText(value)
					else
					subTextOut := value
				}
				Case "subBkgdColour":
				{
					if (value != -1)
					{
						if (This.updateFlag > 0)
						This.subBkgdColour := This.ValidateColour(value, 1)
						else
						subBkgdColourOut := value
					}
				}

				Case "subFontName":
				{
					if (value)
					{
						if (This.updateFlag > 0)
						This.subFontName := This.ValidateText(value)
						else
						subFontNameOut := value
					}
				}
				Case "subFontSize":
				{
					if (200 >= value >= 0) ; arbitrary limit
					{
						if (This.updateFlag > 0)
						This.subFontSize := Floor(value)
						else
						subFontSizeOut := value
					}
				}
				Case "subFontWeight":
				{
					if (1000 >= value >= 0)
					{
						if (This.updateFlag > 0)
						This.subFontWeight := Floor(value)
						else
						subFontWeightOut := value
					}
				}
				Case "subFontColour":
				{
					if (value != -1)
					{
						if (This.updateFlag > 0)
						This.subFontColour := This.ValidateColour(value, 1)
						else
						subFontColourOut := value
					}
				}
				Case "subFontQuality":
				{
					if (value >= 0 && value <= 5)
					{
						if (This.updateFlag > 0)
						This.subFontQuality := Floor(value)
						else
						subFontQualityOut := value
					}
				}
				Case "subFontItalic":
				{
					if (This.updateFlag > 0)
					This.subFontItalic := (value)? " Italic": ""
					else
					subFontItalicOut := value
				}
				Case "subFontStrike":
				{
					if (This.updateFlag > 0)
					This.subFontStrike := (value)? " Strike": ""
					else
					subFontStrikeOut := value
				}
				Case "subFontUnderline":
				{
					if (This.updateFlag > 0)
					This.subFontUnderline := (value)? " Underline": ""
					else
					subFontUnderlineOut := value
				}


				}
				
			}
		}

	This.SplashImgInit(parentOut, imagePathOut, imageUrlOut
	, bkgdColourOut, transColOut, vHideOut, noHWndActivateOut
	, vOnTopOut, vMovableOut, vBorderOut, vImgTxtSizeOut
	, vPosXOut, vPosYOut, vMgnXOut, vMgnYOut, vImgWOut, vImgHOut
	, mainTextOut, mainBkgdColourOut
	, mainFontNameOut, mainFontSizeOut, mainFontWeightOut, mainFontColourOut
	, mainFontQualityOut, mainFontItalicOut, mainFontStrikeOut, mainFontUnderlineOut
	, subTextOut, subBkgdColourOut
	, subFontNameOut, subFontSizeOut, subFontWeightOut, subFontColourOut
	, subFontQualityOut, subFontItalicOut, subFontStrikeOut, subFontUnderlineOut)
	
	}

	SplashImgInit(parentIn, imagePathIn, imageUrlIn
	, bkgdColourIn, transColIn, vHideIn, noHWndActivateIn
	, vOnTopIn, vMovableIn, vBorderIn, vImgTxtSizeIn
	, vPosXIn, vPosYIn, vMgnXIn, vMgnYIn, vImgWIn, vImgHIn
	, mainTextIn, mainBkgdColourIn
	, mainFontNameIn, mainFontSizeIn, mainFontWeightIn, mainFontColourIn
	, mainFontQualityIn, mainFontItalicIn, mainFontStrikeIn, mainFontUnderlineIn
	, subTextIn, subBkgdColourIn
	, subFontNameIn, subFontSizeIn, subFontWeightIn, subFontColourIn
	, subFontQualityIn, subFontItalicIn, subFontStrikeIn, subFontUnderlineIn)
	/*
	; Future expansion for vertical text:
	, rightText := "", rightFontNameIn := "", rightFontSizeIn := 0, rightFontWeightIn := 0, rightFontColourIn := -1
	, leftText := "", leftFontNameIn := "", leftFontSizeIn := 0, leftFontWeightIn := 0, leftFontColourIn := -1
	; also consider transparency
	*/
	{
	vWinW := 0, vWinH := 0, parentW := 0, parentH := 0, init := 0
	mainTextSize := [0, 0], subTextSize := [0, 0]
	currVPos := {x: "", y: ""}
	static splashyInst := ""
	; Border constants
	Static WS_DLGFRAME := 0x400000, WS_CAPTION := 0xC00000, WS_POPUP := 0x80000000, WS_CHILD := 0x40000000, WS_EX_COMPOSITED := 0X2000000
	Static WS_EX_WINDOWEDGE := 0x100, WS_EX_STATICEDGE := 0x20000, WS_EX_CLIENTEDGE := 0x200, WS_EX_DLGMODALFRAME := 0x1

	; Determines redraw of Splashy window (placeholder)
	diffPicOrDiffDims := 0


	This.procEnd := 0

		if (This.updateFlag <= 0)
		{
		;Set defaults

		This.parent := parentIn

			if (StrLen(imagePathIn))
			This.imagePath := imagePathIn
			else
			{
				if (!This.imagePath && !StrLen(imageUrlIn))
				This.imagePath := A_AhkPath ; default icon. Ist of 5
			}

			if (StrLen(imageUrlIn))
			This.imageUrl := imageUrlIn
			else
			{
				if (!This.imageUrl)
				This.imageUrl := "https://www.autohotkey.com/assets/images/features-why.png"
			}


			if (bkgdColourIn == -1)
			{
				if (This.bkgdColour == "")
				This.bkgdColour := This.GetDefaultGUIColour()
			}
			else
			This.bkgdColour := This.ValidateColour(bkgdColourIn)


		This.transCol := transColIn

		This.vHide := vHideIn

			if (noHWndActivateIn)
			This.noHWndActivate := "NoActivate "
			else
			This.noHWndActivate := ""

		This.vOnTop := vOnTopIn
		This.vMovable := vMovableIn
		This.vBorder := vBorderIn
		This.vImgTxtSize := vImgTxtSizeIn

		This.vPosX := (vPosXIn == "")? This.vPosX: vPosXIn
		This.vPosY := (vPosYIn == "")? This.vPosY: vPosYIn
		This.vMgnX := (vMgnXIn == "")? This.vMgnX: vMgnXIn
		This.vMgnY := (vMgnYIn == "")? This.vMgnY: vMgnYIn

			if (vImgWIn > 0)
			This.inputVImgW := vImgWIn
			else
			{
				if (vImgWin <= -10)
				This.inputVImgW := 0
				else
				This.inputVImgW := vImgWIn
				; a zero default can be Floor(A_ScreenWidth/3)
			}
			if (vImgHIn > 0)
			This.inputVImgH := vImgHIn
			else
			{
				if (vImgHin <= -10)
				This.inputVImgH := 0
				else
				This.inputVImgH := vImgHIn
				; a zero default can be Floor(A_ScreenHeight/3)
			}




			if (StrLen(mainTextIn))
			This.mainText := This.ValidateText(mainTextIn)
			else
			This.mainText := ""

			if (mainBkgdColourIn == -1)
			{
				if (This.mainBkgdColour == "")
				This.mainBkgdColour := This.GetDefaultGUIColour()
			}
			else
			This.mainBkgdColour := This.ValidateColour(mainBkgdColourIn, 1)

			if (StrLen(mainFontNameIn))
			This.mainFontName := This.ValidateText(mainFontNameIn)
			else
			{
				if (!This.mainFontName)
				This.mainFontName := "Verdana"
			}

			if (mainFontSizeIn)
			This.mainFontSize := Floor(mainFontSizeIn)
			else
			{
				if (!This.mainFontSize)
				This.mainFontSize := 12
			}

			if (mainFontWeightIn)
			This.mainFontWeight := Floor(mainFontWeightIn)
			else
			{
				if (!This.mainFontWeight)
				This.mainFontWeight := 600
			}

			if (mainFontColourIn == -1)
			{
				if (This.mainFontColour == "")
				This.mainFontColour := This.GetDefaultGUIColour(1)
			}
			else
			This.mainFontColour := This.ValidateColour(mainFontColourIn, 1)

			if (mainFontQualityIn >= 0)
			This.mainFontQuality := Floor(mainFontQualityIn)
			else
			{
				; NONANTIALIASED_QUALITY for better performance
				; https://stackoverflow.com/questions/8283631/graphics-drawstring-vs-textrenderer-drawtextwhich-can-deliver-better-quality/23230570#23230570
				if (!This.mainFontQuality)
				This.mainFontQuality := 1
			}

		This.mainFontItalic := (mainFontItalicIn)? " Italic": ""

		This.mainFontStrike := (mainFontStrikeIn)? " Strike": ""

		This.mainFontUnderline := (mainFontUnderlineIn)? " Underline": ""



			if (StrLen(subTextIn))
			This.subText :=  This.ValidateText(subTextIn)
			else
			This.subText := ""

			if (subBkgdColourIn == -1)
			{
				if (This.subBkgdColour == "")
				This.subBkgdColour := This.GetDefaultGUIColour()
			}
			else
			This.subBkgdColour := This.ValidateColour(subBkgdColourIn, 1)


			if (StrLen(subFontNameIn))
			This.subFontName := This.ValidateText(subFontNameIn)
			else
			{
				if (!This.subFontName)
				This.subFontName := "Verdana"
			}

			if (subFontSizeIn)
			This.subFontSize := Floor(subFontSizeIn)
			else
			{
				if (!This.subFontSize)
				This.subFontSize := 10
			}

			if (subFontWeightIn)
			This.subFontWeight := Floor(subFontWeightIn)
			else
			{
				if (!This.subFontWeight)
				This.subFontWeight := 400
			}

			if (subFontColourIn == -1)
			{
				if (This.subFontColour == "")
				This.subFontColour := This.GetDefaultGUIColour(1)
			}
			else
			This.subFontColour := This.ValidateColour(subFontColourIn, 1)

			if (subFontQualityIn >= 0)
			This.subFontQuality := Floor(subFontQualityIn)
			else
			{
				if (!This.subFontQuality)
				This.subFontQuality := 1
			}


		This.subFontItalic := (subFontItalicIn)? " Italic": ""

		This.subFontStrike := (subFontStrikeIn)? " Strike": ""

		This.subFontUnderline := (subFontUnderlineIn)? " Underline": ""

			if (This.updateFlag == -1) ; init
			{
			This.updateFlag := 1
			init := 1
			}
		}

		if (diffPicOrDiffDims := This.GetPicWH())
		{
			if (diffPicOrDiffDims) == "error"
			return
			else
			{
				if (This.DisplayToggle() == "error")
				return
			}
		}

	DetectHiddenWindows On
	splashyInst := "Splashy" . (This.instance)

		if (!This.hWndSaved[This.instance])
		{
			if (!This.hGDIPLUS)
			{
				if (This.hGDIPLUS := DllCall("LoadLibrary", "Str", "GdiPlus.dll", "Ptr"))
				{
				VarSetCapacity(SI, (A_PtrSize = 8 ? 24 : 16), 0), Numput(1, SI, 0, "Int")
				DllCall("GdiPlus.dll\GdiplusStartup", "UPtr*", spr, "Ptr", &SI, "Ptr", 0)
				; for return value see status enumeration in  gdiplustypes.h 
				This.pToken := spr
				}
				else
				msgbox, 8208, LoadLibrary, Critical GDIPLUS error!
			}

		;Create Splashy window

		Gui, %splashyInst%: New, +OwnDialogs +ToolWindow -Caption -DPIScale +E%WS_EX_COMPOSITED% ;  WS_POPUP active since default

		This.NewWndObj := new Splashy.NewWndProc(This.instance)
		}

		; Set borders:
		if (This.voldBorder || This.vBorder) ; null or zero
		{
			if (This.instance != This.oldInstance || This.voldBorder != This.vBorder)
			{
			; -0x800000 is not sufficient to remove the standard borders.
			Gui, %splashyInst%: -%WS_CAPTION% -E%WS_EX_WINDOWEDGE% -E%WS_EX_STATICEDGE% -E%WS_EX_CLIENTEDGE% -E%WS_EX_DLGMODALFRAME%
				if (This.vBorder)
				{
					if (This.vBorder == "b")
					Gui, %splashyInst%: +Border
					else
					{
						if (This.vBorder == "dlgframe")
						Gui, %splashyInst%: +WS_DLGFRAME
						else
						{
							Loop, Parse, % This.vBorder
							{
								Switch (A_Loopfield)
								{
									Case "w":
									Gui, %splashyInst%: +E%WS_EX_WINDOWEDGE%
									Case "s":
									Gui, %splashyInst%: +E%WS_EX_STATICEDGE%
									Case "c":
									Gui, %splashyInst%: +E%WS_EX_CLIENTEDGE%
									Case "d":
									Gui, %splashyInst%: +E%WS_EX_DLGMODALFRAME%
								}
							}
						}
					}
				}

			This.voldBorder := This.vBorder
			}
		}

		if (spr := This.parentHWnd)
		{
			if (This.parent)
			{

			; Somehow co-ordinates go wrong if the position is not obtained here
				if (!init)
				currVPos := This.GuiGetPos(This.hWnd(), 1)

			Gui, %splashyInst%: -%WS_POPUP% +%WS_CHILD%
			Gui, %splashyInst%: +parent%spr%

			point := This.GuiGetPos(spr)

			parentW := point.w
			parentH := point.h

				if (This.parentClip)
				Winset, Style, % -This.parentClip , % "ahk_id" This.parentHWnd
			point := ""
			}
			else
			{
			Gui, %splashyInst%: -parent%spr%
			Gui, %splashyInst%: +%WS_CHILD% +%WS_POPUP%
			parentW := A_ScreenWidth
			parentH := A_ScreenHeight
				if (This.parentClip)
				Winset, Style, % This.parentClip , % "ahk_id" This.parentHWnd
			}
		}
		else
		{
		parentW := A_ScreenWidth
		parentH := A_ScreenHeight
		}

	Gui, %splashyInst%: Color, % This.bkgdColour

		if (This.vMgnX == "d")
		{
		SM_CXEDGE := 45
		sysget, spr, %SM_CXEDGE%
		This.vMgnX := spr
		}

		if (This.vMgnX == "d")
		{
		SM_CYEDGE := 46
		sysget, spr, %SM_CYEDGE%
		This.vMgnY := spr
		}


	This.vImgX := This.vMgnX, This.vImgY := This.vMgnY
	vWinW := This.vImgW + 2 * This.vMgnX
	vWinH := This.vImgH + This.vMgnY


		This.vImgY := This.DoText(splashyInst, This.mainTextHWnd[This.instance], This.mainText, currVPos, vWinW, vWinH, init)

		if (spr := This.DoText(splashyInst, This.subTextHWnd[This.instance], This.subText, currVPos, vWinW, vWinH, init, 1))
		vWinH += spr + This.vMgnY


	Gui, %splashyInst%: Font

		; now set hWndSaved[This.instance] in hWnd()
		if (This.vHide)
		WinHide % "ahk_id" This.hWnd()
		else
		{
		spr1 := 0
		spr := A_Space

			if (!(This.Parent && (This.mainText || This.subText)))
			{
			currVPos := This.GetPosVal(This.vPosX, This.vPosY, currVPos, parentW, parentH, vWinW, vWinH, (This.parent?This.parentHWnd:0))
			currVPos := This.GetPosProc(splashyInst, currVPos, init)
			}

			if (This.parent)
			Gui, %splashyInst%: Show, % This.noHWndActivate . Format("X{} Y{} W{} H{}", currVPos.x, currVPos.y, vWinW, vWinH)
			else
			{
			; Also consider cloaking (DWMWA_CLOAK)
			Gui, %splashyInst%: Show, % "Hide " . Format(" W{} H{}", vWinW, vWinH)

			;WinGetPos, point.x, point.y,,, % "ahk_id" . This.hWnd(); fail

			; Supposed to prevent form visibility without picture while loading. Want another approach?
			Gui, %splashyInst%: Show, % "Hide " . Format("X{} Y{}", -30000, -30000)
			sleep 20
			;WinMove, % "ahk_id" . This.hWnd(),, % point.x, % point.y ; fails here whether 30000 or 0, as well as SetWindowPos. SetWindowPlacement?

			Gui, %splashyInst%: Show, % This.noHWndActivate . Format("X{} Y{}", currVPos.x, currVPos.y)
			}


		WinSet, AlwaysOnTop, % (This.vOnTop)? 1 : 0, % "ahk_id" . This.hWnd()
			if (This.transCol && !This.vBorder)
			WinSet, TransColor, % This.bkgdColour, % "ahk_id" . This.hWnd()
		Splashy.NewWndProc.PaintProc(This.hWnd())
		}

	This.procEnd := 1
	This.oldInstance := This.instance
	SetWorkingDir % This.userWorkingDir
	DetectHiddenWindows Off

	; Sleep -1 is critical, else PaintDC gets out of sync
	Sleep, -1

	}
	;==========================================================================================================
	;==========================================================================================================


	ValidateText(string)
	{
		if (StrLen(string))
		{
			if (StrLen(string) > 20000) ;length?
			string := SubStr(string, 1, 20000)
		}
		else ; "0", or some irregularity in string.
		{
		string =
		}
		return string
	}

	ValidateColour(keyOrVal, toBGR := 0)
	{


		if (This.HTML.HasKey(keyOrVal))
		{
		keyOrVal := This.ToBase(This.HTML[keyOrVal], 16)
		spr1 := StrLen(keyOrVal)
		}
		else
		{
		spr := ""

		spr1 := StrLen(keyOrVal)

		; If "0X" found, remove it. Will be added later. Remove other X's
		
		if (InStr(SubStr(keyOrVal, 1, 2), "0X"))
		keyOrVal := StrReplace(SubStr(keyOrVal, 3, spr1 - 2), "X", "0")
		else
		keyOrVal := StrReplace(SubStr(keyOrVal, 1, spr1), "X", "0")			; filter out numerics 

			if keyOrVal is not xdigit
			{
				; Filter out all but numerics
				loop, Parse, keyOrVal, , %A_Space%%A_Tab% `,
				{
					if A_Loopfield is xdigit
					spr .= A_Loopfield
				}

				if (spr)
				keyOrVal := spr
				else
				keyOrVal := 0
			}

			if (spr1 != 6 && keyOrVal is digit) ;  assume decimal,
			; which may not be desired if they were digits in above loop
			keyOrVal := This.ToBase(keyOrVal, 16)

		spr1 := StrLen(keyOrVal)

			if (spr1 > 8)
			spr1 := 8
			else
			{
				if (spr1 < 3)
				return "0X0"
			}

		}

	; pad zeroes
	spr2 := ""

		loop, % (6 - spr1)
		spr2 := spr2 . "0"

	spr := "0X" . spr2 . keyOrVal


		if (toBGR) ; for the GDI functions (ColorRef)
		spr := This.ReverseColour(spr)
		else
		{
			if (InStr(spr, "0X"))
			spr := SubStr(spr, 3, 6) ; "0X" prefix not required for AHK gui functions
		}

	return spr
	}

	ReverseColour(colour)
	{

		colour := ((colour & 0x0000FF) << 16 ) | (colour & 0x00FF00) | ((colour & 0xFF0000) >> 16)

		if colour is digit
		{
		spr := This.ToBase(colour, 16)
		; possible to return spr here
		; The following just pads the zeroes.
		spr1 := StrLen(spr)

		spr2 := ""

		loop, % (6 - spr1)
		spr2 := spr2 . "0"

		colour := "0X" . spr2 . spr

		}
		return colour
	}

	ToBase(n, b)
	{
	; Hex numbers n must be in quoted "0Xn" format
		Loop
		{
		r := mod(n, b)
		d := floor(n/b)

			if (b == 16)
			r := (r > 9)? chr(0x37 + r): r

		m := r . m
		n := d
			if (n < 1)
			Break
		}
	; returns without "0X"
	Return m
	}

	BinToHex(P, Bytes := 4, Prefix := "")
	{ 
	spr := ""
	Loop % (Bytes)
	   spr .= Format( "{:02X}", *(P + A_Index-1))
	Return ( Prefix . spr )
	}

	GetDefaultGUIColour(font := 0)
	{
		static COLOR_WINDOWTEXT := 8, COLOR_3DFACE := 15 ;(more white than grey these days)

		spr := DllCall("User32.dll\GetSysColor", "Int", (font)? 8: 15, "UInt")
		;BGR +> RGB
		spr := This.ReverseColour(spr)

		return spr
	}

	DownloadFile(URL, fName)
	{
			try
			{
;https://webapps.stackexchange.com/questions/162310/url-image-filtering-url-suffixes-and-wikimedia
			;SplitPath,d,name
			;UrlDownloadToFile,%d%,%name%
				UrlDownloadToFile, %URL%, %fName%
			}
			catch spr
			{
			msgbox, 8208, FileDownload, Error with the bitmap download!`nSpecifically: %spr%
			}		
		FileGetSize, spr , % A_ScriptDir . "`\" . fName
			if spr < 50 ; some very small image
			{
			msgbox, 8208, FileDownload, File size is incorrect!
			return 0
			}
			sleep 50
			return 1


			return fName

	}


	DisplayToggle()
	{
	static vToggle := 1
	spr := 0, spr1 := 0
	; This function uses LoadPicture to populate hBitmap and hIcon
	; and sets the image type for the painting routines accordingly
	; Now that the image dimensions are determined from GetPicWH,
	; adjust the images with the extra possible input from vImgW and vImgW.

	vToggle := !vToggle
	; If the image is the same between calls, this routine is never called, 
	; so vToggle will not update.

	; This.inputVImgW == "" is valid for This.vImgTxtSize
	if (This.inputVImgW == "")
	This.vImgW := (This.vImgW? This.vImgW: This.actualVImgW)
	else
	{
		; In case vImgW is specified in the first call of the image,
		if (This.vImgW)
		This.vImgW := (This.inputVImgW? This.inputVImgW: This.actualVImgW)
		else
		This.vImgW := This.ProcImgWHVal((This.inputVImgW)?This.inputVImgW:This.actualVImgW)
	}

	if (This.inputVImgH == "")
	This.vImgH := (This.vImgH? This.vImgH: This.actualVImgH)
	else
	{
		if (This.vImgH)
		This.vImgH := (This.inputVImgH? This.inputVImgH: This.actualVImgH)
		else
		This.vImgH := This.ProcImgWHVal((This.inputVImgH)?This.inputVImgH:This.actualVImgH, 1)
	}








	spr1 := Format("W{} H{}", spr, spr1)

		if (This.imagePath)
		{
			if (This.hWndSaved[This.instance])
			{
				This.oldImagePath := This.imagePath

				if (This.hBitmap)
				{
					This.DeleteHandles()
					This.oldPicInScript := 0
				}
				else
				{
					if (This.hIcon)
					{
						if (This.picInScript)
						This.oldPicInScript := 1
						else
						This.DeleteHandles()
					}
				}
			}
		SplitPath % This.imagePath,,, spr

			if (StrLen(spr))
			This.vImgType := ((spr == "cur")? 2: (spr == "exe" || spr == "ico")? 1: 0)
			else
			This.vImgType := 0 ; assume image



			if (InStr(This.imagePath, "*"))
			{
			This.vImgType := 1
			return
			}
			else
			{
				if (fileExist(This.imagePath))
				{

				spr := This.imagePath

					if (This.vImgType)
					{
						if (This.imagePath == A_AhkPath)
						{
						This.hIcon := LoadPicture(A_AhkPath, ((vToggle)? "Icon2": "") . spr1, spr)
						return
						}
						else
						{
							if (This.hIcon := LoadPicture(spr, spr1, spr)) ; must use 3rd parm or bitmap handle returned!
							return
						}
					}
					else
					{

						if (This.hBitmap := LoadPicture(spr, spr1))
						return
					}
				}
				else
				{
					if (!fileExist(This.ImageName) && !This.hIcon)
					{
					msgbox, 8208, DisplayToggle, Unknown Error!
					return "error"
					}
				}
			}
		SplitPath % This.imagePath, spr
		This.ImageName := spr
		}
		else
		This.oldImagePath := ""

		; Fail, so download

			if (This.imageUrl && RegExMatch(This.imageUrl, "^(https?://|www\.)[a-zA-Z0-9\-\.]+\.[a-zA-Z]{2,3}(/\S*)?$"))
			{
				if (!(This.ImageName))
				{
				SplitPath % This.imageUrl, spr
				This.ImageName := spr
				}
				;  check if file D/L'd previously
				for key, value in % This.downloadedUrlNames
				{
					if (This.imageUrl == value)
					{
						if (fileExist(key))
						{
							Try
							{
								if (key != This.ImageName)
								FileCopy, %key%, % This.ImageName
							Break
							}
							Catch e
							{
							msgbox, 8208, FileCopy, % key . " could not be copied with error: " . e
							}
						}
					}
				}


				if (!fileExist(This.ImageName))
				{
					if (!(This.DownloadFile(This.imageUrl, This.ImageName)))
					return "error"
				}

				if (This.hBitmap := LoadPicture(This.ImageName, spr1))
				{
				This.oldImageUrl := This.imageUrl
				This.vImgType := 0
				spr := This.ImageName

				This.downloadedPathNames.Push(spr) 
				This.downloadedUrlNames(spr) := This.oldImageUrl
				return
				}
				else
				{
				msgbox, 8208, LoadPicture, Format not recognized!
				FileDelete, % This.ImageName
				return "error"
				}

			}
			else
			spr := 1		

		; "Neverfail" default 
		This.hIcon := LoadPicture(A_AhkPath, ((vToggle)? "Icon2 ": "") . spr1, spr)
		This.vImgType := 1
		
	}

	GetPicWH()
	{
	Static ICONINFO := [], vToggle := 1

	vToggle := !vToggle
	/*
	typedef struct tagBITMAP {
	  LONG   bmType;
	  LONG   bmWidth;
	  LONG   bmHeight;
	  LONG   bmWidthBytes;
	  WORD   bmPlanes; // Short
	  WORD   bmBitsPixel; // Short
	  LPVOID bmBits; // FAR ptr to void
	} BITMAP, *PBITMAP, *NPBITMAP, *LPBITMAP; ==> Extra pointer reference
	*/

	if (This.imagePath)
	{
		if (This.hWndSaved[This.instance])
		{
			if (This.oldImagePath == This.imagePath)
			{
			; No need to reload
				if (This.inputVImgW && This.inputVImgH)
				{
					if (This.oldVImgW == This.inputVImgW && This.oldVImgH == This.inputVImgH)
					return 0
				}
				else
				{
					if ((This.picInScript && This.oldPicInScript) || (!This.picInScript && !This.oldPicInScript))
					{
						if (This.vImgTxtSize)
						{
							if (This.oldVImgW == This.vImgW)
							{
								; check for height
								if (This.oldVImgH == This.inputVImgH)
								return 0
								else
								This.oldVImgH := This.inputVImgH
							}
							else
							This.oldVImgW := This.vImgW
						}
						else
						{
							if (This.inputVImgW != "") ; else just switched off vImgTxtSize
							{
								if (This.oldVImgW == This.inputVImgW) && (This.oldVImgH == This.inputVImgH)
								return 0
								else
								{
									if (This.oldVImgW != This.inputVImgW && This.oldVImgH == This.inputVImgH)
									{
									This.oldVImgW := This.inputVImgW
									This.oldVImgH := This.inputVImgH
									}
									else
									{
										if (This.oldVImgW != This.inputVImgW)
										This.oldVImgW := This.inputVImgW
										else
										This.oldVImgH := This.inputVImgH
									}
								}
							}
						}
					}
				}
			}
			This.DeleteHandles()
		}

	SplitPath % This.imagePath,,, spr

		if (StrLen(spr))
		This.vImgType := ((spr == "cur")? 2: (spr == "exe" || spr == "ico")? 1: 0)
		else
		This.vImgType := 0 ; assume image



		if (InStr(This.imagePath, "*"))
		{
		This.PicInScript := {}      
		this.SetCapacity(PicInScript, 21748 << !!A_IsUnicode)
		This.PicInScript := "iVBORw0KGgoAAAANSUhEUgAAAPoAAAEACAMAAACtTJvEAAAAAXNSR0IArs4c6QAAAARnQU1BAACxjwv8YQUAAAMAUExURQAAAEO5/0G7/0G8/kG8/0K+/0e5/ka4/0a6/0W9/kS8/0W+/0q3/0y2/k62/0m4/0u6/km6/0q8/0m//ki+/024/026/ky6/028/0++/k6+/1O0/1K2/1ay/1W1/lS0/1S2/1K4/1O6/lG6/1C8/1G+/lG+/1W5/lW4/1W6/1S9/lW8/1S+/1i0/1m2/12y/121/163/l22/1i4/1q6/1m8/1q+/lm+/124/1y6/128/12//l6+/2O0/2G3/2O5/mK4/2G7/mG6/2O8/mC8/2G+/2W4/2S6/2W8/mS8/2W+/2q6/2m8/2m+/266/228/26//my+/3K7/3K8/3G+/0bA/kbA/0rA/knA/0vC/0zA/0/D/k3C/07E/k7E/1PB/lHA/1DC/lDC/1LE/lLE/1PH/1bA/lXA/1bD/lbC/1XE/lTE/1XG/lbG/1bI/lXI/1nA/1nC/1jF/lrE/1nG/17B/l3A/13C/lzC/1/E/l3E/13G/l7G/1rI/lnI/13I/lzI/1zK/l3K/2LB/mHA/2HC/2HE/2HH/mHG/2TB/mXA/2XC/mXC/2XE/2TG/mXG/2DI/mLI/2HK/2HM/2PO/2bJ/mXI/2bK/mbK/2XM/mXM/2bO/mXO/2rA/mjA/2jC/2nE/mnE/2nG/2zA/23C/2zE/23G/2nI/mnI/2nK/2nM/mnM/2nO/mnO/23I/23K/m3K/2/N/m3M/2zP/m3O/2rQ/2rS/mzQ/m3Q/2zS/m7S/3HA/3HC/3LE/3HG/nHG/3bA/3XC/3XE/3XG/3HI/3HK/nHK/3DN/nDM/3DP/3XI/3XK/3jA/3jD/3jE/3nG/33D/33F/33G/3jI/3vK/33I/3DQ/oDD/4LH/4bG/4DI/4XI/4nI/wAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAADwzn70AAAEAdFJOU////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////wBT9wclAAAACXBIWXMAAA7DAAAOwwHHb6hkAAAAGXRFWHRTb2Z0d2FyZQBwYWludC5uZXQgNC4wLjIx8SBplQAAOw9JREFUeF7tfY9zG1me17G2Lp50QVY6z6jpWmCCtY19MUdBHEyMuTqOCccNLLDAKJKicfXEyjUapddqJLK+rrYsdTm6Ll+y3pZ8TcdWjXFYG+ti+ab2RrHWt0vYXd/sksBtcRwBjvtfmu/3dUtqyZLdkpXM7B2fxOqW7MT66Pve931/ve/7GfPPLP4/9T81YO8nk8ndh/azU/Gnifqib8TrGxkZ8Y54L3jz9ovd8aeGet6L8MEfr/eiDz6EC2eR/1NC/TGy9jMMDX/g0eeDT+CSz/5mF5yb+nft62cKIE7TPiDNsCyLFx+D7C/Q9vc74jzUH/un7LvPFve9fpqmgTX8DXBwCQB9H4MT4L79I53QN/U9Xbph337GeOj1MhEGKHMcx/O3eZ6D+wAHYx8Ef8qE74/68cVS4eH37SefNXCkA23+dlxMinFRjCN5Dkb9KO31+SbtnzqJvqiPmub+t+z7zxz0aCDAcnwciQNSqZQYTwJ7GP0046W99o+dRB/UmZppRn5gPzmJA/rh2tra4+6f9mAh0TjSkfOiJKVS8IUfAEqeZSM0KPqk/YMn0Dv12/AV+UPrvh33J0HhMDD4pFgmqPL8q+d/7Kc5DkZ5SpbkOiRZEuNx4B5gGLBx7J88gV6pcxo8LHeWedKHEhDhk1/M2C9R/NlW1bmA0zzOw+9MEeoqYY6DHgTP8Syouq4jvkfqeyV4qP2e9aQVSS98/jx8+pKsaoauJ6yXN+WL1s0rwQUaPmsR6EqapqrwF6DBO5AkmPgw30HsXr/9s+3ojXr5KT6ukvtW5L0My3OiJEmqrpeHLlIhQKyI36pIXyc/8irgZXgt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"
		This.PicInScript .= "wuSAUZLiz4qhdKb+b73+CMOLKRmTaxse+FWGlmJfZTPlDsAomIhVDIYKAtA3DI9nCE9Po4Y8w5jqtA5+xna7SeCO5PFfJRkRflxbk0T2jM1YnakHQL+zfBy4w3+z91v4P0mT/dtNfQGm723pjVAoQgFxo2zguXEUNQpzz+oZiged6ypReETurN3AjktpxhDmv8+qeuhMnfbBNMMBj3lFNY+tL153bm3Sz3BSJBqNhnRMaOOsey9kgwpRnuENTcOVTkrLeMw76jqGTPdJWQ+Fo1RZTZ1hgHWmDp44A1LHBAuMKHgQ475/YH/v9YD2M4wYm79161ZZL5dhnAdB/qFoNByORMJhakNu9o/7cxgXt6Y7lnXel6i5BeFOyPCcsW+wM/ULQJ3D9RTIoyKJ8/R5W831CJjo8ZhwNxYLgZ71UKOhUHguFpubn58HC0Nv75s3AsYe+jNEziIlLOfSdzwmOU24O7pInSYZFst2TIlxlumn8Psc4Pw0H1EyC0KiXB72jMIInp9PxISEkEhEjU5Kh70NYrda0yZDuc3S+kK0P+peHPFgH5OhxHGvPQD32E9PGqWCkkmHjCEc6lijKAhCeiET7VYmuXgbFncsEZ/yGEfm08L8WQeVdaaeBJ+BZiMsBtuZQI+51F2eHvFe8ibP6tl8CkDJpvarpW/m7mKpFog8hm4Enggc7m6k7UXs9W0NS382ldMalCI6UzcDIyB48IlIA99e9lElfb4R8KHJUV/efoMoSS/DZXB7ejGKZjR4zQJhvpwLn0ZnxG7GvIIZk/1as0NpZ3Shbl64iG8fOPh7cQiTGDXzg57AkAp8aF5nT2z38LEMzNOj/Y0sGDSRDxIJIZPN5pSiErXTA4iHbGByYuxK4EqzNDzvIzr+Mta/nNFZFtCNOuAxfaHHJKoPMxMkfMaB/8AGGHj09q4gH4OewSxfpZQA5rGYkMkogEJBaL6f5JWJifHxy+NfevvtsYnG4GJpEqm0S3/OwCnUewbjo/006AZwsC1gAJHx9VxixXpZMq5rJbKkCUImtwzEHymNPe7JK2PjgCkLX5oI1CVv9ZwnebKFf42Pp2CA1LdH6FFL4jzGFnBlxPw7S/c65X0MR2oUX2apm1EY7Eu5YrFQ2CxZLboAI1fGxyemrr8zs3Jj5cbM7MzU1cu2a2X12sZNUNUzC+YHSH3Ei7lnEkRbJN394C8fvw2y9/eUo7lAs7YdFsE+cgsZmOSFUqmyXF9oAoHx8evXZ1dWVx9sfbS1/eD+O1NT9S24ZBnG2aJ8gHenYXDU+RFMPTMcRg/BAE6B34jd0EH0IPhetB0sq9Z+oCPcboHMCxulzU/2GyYcCzKfeZf02X327Hhnd2drdXbc7gTR2PvVGCJdMTjquBzAaJ+MJ7GoDux+DKSAAwCyZzmmh1S0l7bLbhVQcQsZGO2l0tPq0UZdu0+PjV+/8e6D7d1nz59f8vvfPD5+trv6zlWrc0odLjTd4KhfAi+TeLqilMbaaYwkrYHbJ6XBrD7lCJd2SAxnb32cj8YW0tlC8VGpUq3Wblkvmg8Db0/NrD7Y3T320pO/lkzyyUn6050Hs62Jod+wr6dgcNQvwrqGLk8KJC5rmm4VEMMFtbUecW0LL/rYN6y7cCyRySqF9dL+06pZqxso/NtXZ959uPv8j76cXPx17Dv+jfziNH28OuMUu5vdnoOjTpQchxNdBnljLKk8NLSB8QScuTrj1hqGRd0ardVoLJ1ZLhRLIPKjZrO8ifGp2dWdYy+7KOl7v/Nj0/zRpwff/Lr/2YpV4uQeg6OO0Y0AjHeY5ypGFzyeIBUcDVIegywzkluxgxFr3RRjX0tni4X1zcrRkdno/3oAM311+/gSv7hWblb6lRfpnfYkwcGl0yNUg5S6l+QKpDyGTofQxQYEgfvwe/h9ziV3OmDnkQQw43LFzc0j0nPGPnHMfIwdhnefB77+Wy11Tx9//dOVltl+z+/zwUC0n3XCAKljPA/UuyxrxsYQFQQfOzYXm4tEw1QQqy0j7kqoD3ycTR1neqGwXrXOcqmHg+8B9a1n/mRrXSv4bT//wLkLZ9EHWhfcCG93W3Jw1EdgrvO3MUui6kYQmN+O3UksCELi/ehNsiSLrizaSzRje2e3wGlRwJSBiQ6ou49XgPr2cUBqY26a8o4jTcCiy83h+THdranBUT8A6hjATmtrOjrZtwUwvhHpxFwUWygl6DMcaIKHo6y9u3t+QS4apU8soVt1TADp8rXVnRfJtZaKXsTHx81Gu3k6AqbVZBycCF/90LQTGBx1GPFgyllpi+HgzVuCkM6C17GczShCjJhnkhux8zQXIzfVWCZXWK/YdWuNAzn8oOV2LiU7RGCOmzX8DCcm0YzmwX3uKvYBUgexW/sj1vQhKjQPOkoBt2NjQ8vKGQEbc/ycG0U3GWCt3tklVHLrm/apTfVSVVjcrq7svHmvQwioGVGZhMGHYeQUz7WdGufEAKmj0wrOS0rW9HIQ/A5456XSZgm+istpbK1XnOzaG7WJSZq3GrwVsUXzetWuR28MeHNs6t3dtubKFho1VbucKIMVLcsp5N61m9sgqcOSzEyiOacbnpuxBWT+SRXbmZTWlSWY7RU3O/2TDGdRV+4sKeubuKS3YmVidutFsgP1xjkNDDDXDF1F94FrHCp0AgOlvujzsSxQ13RPaD5dLJQ+OcR3flTd31TSm+Yh5YJ6Y65nBSzc2j8ZaBqf2jrmT3rjDfvlmgh2hcfA02UkoP5aBjwYVd5RDs8b1ctUKLFULO3bewiOKhsKzHaBP7vSDaRu9Q9X0tlvPup0UNnDiRu7HU64qzcx+vusNoRJ8WFDVTHX/Jqom4sBWFawqwAVFpYKpcYbP6oWsyVzKX72lvs92opOmVmcMU3qjubREzNbx2L7kU+Nwu4J1RPCVB3mxcUk5zg2rg0Dpg7wg2UTl3UqKih2aS2iWilkzGUXLQmnGc6K0WQyuRzYM+Qe4Fijno3f2PF22NdP8N1UENbVxHyYGtKzUpyjuyaIB0/d/GjK72dFzRPLlhzbx45KIEQ31Wc+1vp8FmBZL1mWHMJ5zBgM+Z23Wob879TX9F/l37uTyS0tJeZCHjxXh+m+IWsg1Pd8a2uPnQL9wbiflY1Yztl65agoKJqLbc91Gz4rAPVux5VN3Nh2LnBNyU5T8K/Wi0pmPjSsSym++1GvA6AuSTeXlExwmPKU95p10r941c/pMcP5zp9kQqqLwL43cJusblkhXagfV4Zo7bj0DA/M4HVkrwealS7TwWJlv7q/DgZkqKzKIkd3711zTuq/L5Kzwiq5THHjt/crldx8I6n9v68G5DtOsT/NRQ0XjSgnGZ7M48yH2ZxzWf839rWOd7dmP9p69mznfzgU4L0oWn1H1VJuYY7C48OI0P8G+d4JnI96kuRxazCLm2/xqNEB8ytjKWc3zJfGvO6C+kPa0nPWut5lwHfE7y3aTaOfFrOJ8BAw59oT8U6cizrxsV4K2XrLyTrqkbH/flmy7HELG4mym2QEM5nEtv5LCZC6k/oZRRLmT9bqs6NSzMSoPU3iyVZjsiOzA85DncF2LZFYtt6t+ST+ZGzN8b1q2jjr7SO8XAL/48y8AI6be6mvNnq5v9xfTgSH34+kGLSgvv8KqDOcWjGCc0KxK3PT/D9jjlNSjzJDbprO8uD1w8gVYulcwdlGtyXA3G4g7DimVklJR+WX39RJyvn3JnALcif0Tx2sdTBYgzFlo/n2tOP2vQLfv+I4k0Kh3FBfg5UdFu25sCDXu3iegestLkqlIBThcy4wpD/k5a6B2r6pv4UtK9BmK27a1I/2Ws64s0Myv+wwPQquqOPKnlKDnnAsU9jsYcTbqCjYv/BQtpLbY5e6HqTeN3UmwMuq7okKuU0rLUp16b820TxLQvG4ikzmaV6UNfD5F5aLrZ7bGdUxgMMSadw4z1uxjGnfZaubfQf0TR0Plpb1YDSRU/ZrtVpB61Y7/IdNuoXymZlfAhoGlGpQ0USmWGkd8Wek7r5TIrpOkGzr9b7oc+7RakW/1A+YCDpoOCpzRUXJqN0Ps5xq9NJTdHdZR56Np/OGJ/RhZnm9zWF3VJRYkKzABqBSynwYOgLzofFL9srxS1Y7907olzrrI1L3wGQXFoSY3u1UUcAfN4ap67M+vKwoq8Zo9K5SrJ95UUcnj/9DfeiNNzx4dntcaqqW7XRaZ+unEHRAv9QDpB+TrlNUKBoJNU+6WrmCJS6XxyYdQ7uh9bPNo4NOBwfrm6oNhe9klBMGndt0NW/Woin/v7KfdUC/1Md8HEhd1vXyEAUuki30rStXx6emrl2bent8oulNfbU+XxNn1C82AbM9rRlUKJEtlppOfw94hpvPdf6Uwdg39TV/gMNj5FXcjyDb+yO2p2ZurK6uPlidnZ2aGB9rRBfq3mXEnZYD5GluMa8N4Ra79f2e1zfLlN4fFU/dDdAv9V0fqHgxhWUTsmTvj3hwfWV7d/eHx892t1ffvX718pU6d7t+rOp2qgNwAQGx45B3xHpcwmr7HNPo04TeN3Ws5STZRTElTrKE21+Y2Xrmn+T5wFsvnh08WJ2ZujxmJwrs9G/BTeKpDrSY1A0c8kozxNcLMjp3etP4vqmvkbpx3GHI2NUiM9svkg/Le/rDxemx5ztbqzNXx+wYgt0Q+2uuxzvggIX5pJdH5wWwa/rgXtiLn1HH3jd1kx/BNC7LBmjLE916YLdWN8uqGHixs3Vj5heuWJPNPgKgF+amOcneTkkaFZxLL5d6n+6l4Jktbvunbi6SjeSNTVY3XjSoffyNe6SyZ+qy9cFjP054N71Rh+kOprKBh6znGgF9tyiEz+73eQ7qbdhy+Cmfqsk3j7duXBu3Pvk/Jo8uzvpoBY0JPJ0Kx0Duzai0GyhB8ewA6MCoT7ywbwjK0uQxiP3tCaLorFNuexQ6AG153QBjeUHpZXnfX6DcHMQzMOrPW0L9P9InX+yuzk6NkVdJF7O7+NAjWIZYy8GYsFxoc2S64qh4e088O9UxQOo/bA0W64tvPXvwjm3TkfzA2ZWbHcDDmJe0MjUnLCmPKidTjyfwcjP9nhZ31cRicAO+tbjl48W3DrZmr14mHiuOenLwRe8QwZqXVMxeZnDQnzHqaxu5iCGxHeoOOmBg1Ldap3J5cQz0nF20i8GKHiy5FvxFnPCaPhyMAPlC5egU8k+LS3PDmugqEgQYGPW2TTYff53eXZ0ZJ6VsfwG+2k4g7gVX2HhcVXVwEReA/H6X0Hy1lIuFPKrIud6wMjjqb9lXC+Xk853Va+NE0cIK6wzH9ww/GPQy9icPRQVSWlQ9apn1R9/ZLyrC+5Shp3o5a2tw1FtPT9b5T7dWrpLxbp17fS48BvMGvUQKD6LIgOxLG5XN/Wqlul+pFAu5dCx6U9dUkW/Jvp6FwVFv9GlD/Fj173x0fZy4NfVThs+FPQYcRSzENDzByHxMSAu5pUwmLSRiH4R/DvctS3z3c9c7Y4DUnfityefbs+PEjO3vdP6T+PVJ7OGKp4TqBu7vpYJU0DOs67qal8Q4y7fYFW7waqjrk8cPb0y1FucPANO43VJMLZK+zVJWlVSZFMdxp2TRu2Ow1O25tsb+cOvGlOW63HYmWweA6UWGgU8AdxdxPEszdM9H6dUxYKk/9C8u8v5n27NT45YgelI8PcGd3XIKBj/gQeDX3rHrs7/S7EH8+cMrUnM/Dfj/1P8s4s8sddP8fy+DYUFVeEvjAAAAAElFTkSuQmCC"

			if ((This.hIcon := This.b64Decode(This.PicInScript))) ; Reqires "HICON:*" for Gui, Add, Picture
			This.vImgType := 1
			else
			{
			; Then try the fallback
			This.imagePath := ""
			This.vImgType := 0
			}
		}
		else
		{
			if (fileExist(This.imagePath))
			{
			spr := This.imagePath

				if (This.vImgType)
				{
					if (This.imagePath == A_AhkPath)
					{
						if (!(This.hIcon := LoadPicture(A_AhkPath, ((vToggle)? "Icon2 ": ""), spr)))
						{
						msgbox, 8208, LoadPicture, Problem loading AHK icon!
						return "error"
						}
					}
					else
					{
						if (!(This.hIcon := LoadPicture(spr, , spr))) ; must use 3rd parm or bitmap handle returned!
						{
						msgbox, 8208, LoadPicture, Problem loading icon!
						return "error"
						}
					}
				}
				else
				{
					if (!(This.hBitmap := LoadPicture(spr)))
					{
					msgbox, 8208, LoadPicture, Problem loading picture!
					return "error"
					}
				}
			}
			else
			This.imagePath := ""
		}
	}
	else
	This.DeleteHandles()


	if (!(This.hBitmap || This.hIcon))
	{

		if (This.ImageName)
		{
		SplitPath % This.imagePath, spr
		This.ImageName := spr
		}

		if (This.imageUrl && RegExMatch(This.imageUrl, "^(https?://|www\.)[a-zA-Z0-9\-\.]+\.[a-zA-Z]{2,3}(/\S*)?$"))
		{
			if (!(This.ImageName))
			{
			SplitPath % This.imageUrl, spr
				if (InStr(spr, ":"))
				{
				msgbox, 8208, Image Url, Name contains a colon, thus not a valid image target!
				return "error"
				}
				else
				This.ImageName := spr
			}
			;  check if file D/L'd previously
			for key, value in % This.downloadedUrlNames
			{
				if (This.imageUrl == value)
				{
					if (fileExist(key))
					{
						Try
						{
							if (key != This.ImageName)
							FileCopy, %key%, % This.ImageName
						Break
						}
						Catch e
						{
						msgbox, 8208, FileCopy, % key . " could not be copied with error: " . e
						return "error"
						}
					}
				}
			}

		; Proceed to download
			if (!fileExist(This.ImageName))
			{
				if (!(This.DownloadFile(This.imageUrl, This.ImageName)))
				return "error"
			}

			if (This.hBitmap := LoadPicture(This.ImageName))
			{
			This.vImgType := 0
			spr := This.ImageName

			This.downloadedPathNames.Push(spr) 
			This.downloadedUrlNames(spr) := This.imageUrl
			}
			else
			{
			msgbox, 8208, LoadPicture, Format of bitmap not recognized!
			FileDelete, % This.ImageName
			return "error"
			}

		}
		else
		spr := 1		


	; "Neverfail" default 
		if (!This.hBitmap)
		{
			if (This.hIcon := LoadPicture(A_AhkPath, ((vToggle)? "Icon2 ": ""), spr))
			This.vImgType := 1
			else
			{
			msgbox, 8208, LoadPicture, Format of icon/cursor not recognized!
			return "error"
			}
		}
	}

	Switch This.vImgType
	{
		case 0:
		{
		bm := []
		spr := (A_PtrSize == 8)? 32: 24
		VarSetCapacity(bm, spr, 0) ;tagBitmap (24: 20) PLUS pointer ref to pBitmap 

			if (!(DllCall("GetObject", "Ptr", This.hBitmap, "uInt", spr, "Ptr", &bm)))
			{
			msgbox, 8208, GetObject hBitmap, Object could not be retrieved!
			VarSetCapacity(bm, 0)
			return "error"
			}

		spr := NumGet(bm, 4, "Int")
		spr1 := NumGet(bm, 8, "Int")
		}
		case 1, 2:
		{
			if (InStr(This.imagePath, "*"))
			{
			; Just get header info
			bm := subStr(This.PicInScript, 1, 100)
			;https://www.autohotkey.com/boards/viewtopic.php?f=6&t=36455&p=168124#p168124

			; CRYPT_STRING_BASE64 := 0x00000001
				if !DllCall("Crypt32.dll\CryptStringToBinary", "Ptr", &bm, "UInt", 0, "UInt", 0x00000001, "Ptr", 0, "UInt*", DecLen, "Ptr", 0, "Ptr", 0)
				return "error"
			VarSetCapacity(spr1, 128), VarSetCapacity(spr1, 0), VarSetCapacity(spr1, DecLen, 0)
				If !DllCall("Crypt32.dll\CryptStringToBinary", "Ptr", &bm, "UInt", 0, "UInt", 0x01, "Ptr", &spr1, "UInt*", DecLen, "Ptr", 0, "Ptr", 0)
				return "error"

			FileAppend , , spr.bin
			tmp := FileOpen("spr.bin", "w")
				if (tmp == 0)
				return "error"
				if (!tmp.RawWrite(&spr1, Declen))
				return "error"
			tmp.Close
			tmp := FileOpen("spr.bin", "r")
			bm := ""
			VarSetCapacity(bm, 24)
				if (!tmp.RawRead(bm, 24))
				return "error"

			VarSetCapacity(spr1, 0)
			tmp.Close
			FileDelete, spr.bin

			spr := This.BinToHex(&bm + 16, 4, "0x")
			spr1 := This.BinToHex(&bm + 20, 4, "0x")

			spr := This.ToBase(spr, 10)
			spr1 := This.ToBase(spr1, 10)
			}
			else
			{
			ICONINFO := []
			tmp := (A_PtrSize == 8)? 104: 84, 0
			; https://www.autohotkey.com/boards/viewtopic.php?t=36733
			; easier way to get icon dimensions is use default SM_CXICON, SM_CYICON
			VarSetCapacity(ICONINFO, (A_PtrSize == 8)? 28: 20, 0) ; ICONINFO Structure
				If (DllCall("GetIconInfo", "Ptr", This.hIcon, "Ptr", &ICONINFO))
				{
					if (ICONINFOhbmMask := NumGet(ICONINFO, (A_PtrSize == 8)? 16: 12, "Ptr"))
					{
					VarSetCapacity(bm, tmp, 0) ; hbmMask dibsection

					DllCall("GetObject", "Ptr", ICONINFOhbmMask, "Int", tmp, "Ptr", &bm)
					spr := NumGet(bm, 4, "UInt")

						; Check for the hbmColor colour plane
						if (ICONINFOhbmColor := NumGet(ICONINFO, (A_PtrSize == 8)? 20: 16, "Ptr"))
						spr1 := NumGet(bm, 8, "UInt")
						else ; The following has the effect of reducing the icon size by exactly half- is that wanted?
						spr1 := NumGet(bm, 8, "UInt")/2

					This.deleteObject(ICONINFOhbmMask)

						if (ICONINFOhbmColor)
						This.deleteObject(ICONINFOhbmColor)
					}
					else
					{
					msgbox, 8208, hbmMask, Icon info could not be retrieved!
					VarSetCapacity(bm, 0)
					return "error"
					}

				}
				else
				; The fastest way to convert a hBITMAP to hICON is to add it to a hIML and retrieve it back as a hICON with COMCTL32\ImageList_GetIcon()
				{
				msgbox, 8208, GetIconInfo, Icon info could not be retrieved!
				VarSetCapacity(bm, 0)
				return "error"
				}
				
			VarSetCapacity(ICONINFO, 0)
			}
		VarSetCapacity(bm, 0)
		}
	}

	This.actualVImgW := spr
	This.actualVImgH := spr1
	return 1
	}

	PaintDC()
	{
	;===============
	static IMAGE_BITMAP := 0, SRCCOPY = 0x00CC0020
	hBitmapOld := 0
	;draw bitmap/icon onto GUI & call GetDC every paint

	This.hDCWin := DllCall("user32\GetDC", "Ptr", This.hWndSaved[This.instance], "Ptr")
		Switch This.vImgType
		{
			case 0:
			{
				if (!(hDCCompat := DllCall("gdi32\CreateCompatibleDC", "Ptr", This.hDCWin, "Ptr")))
				msgbox, 8208, Compat DC, DC could not be created!
				if (hBitmapOld := This.SelectObject(hDCCompat, This.hBitmap, "Bitmap"))
				{
					if (This.oldVImgW || This.oldVImgH || (This.actualVImgW != This.vImgW) || (This.actualVImgH != This.vImgH))
					{
						if (!DllCall("gdi32\StretchBlt", "Ptr", This.hDCWin, "Int", This.vImgX, "Int", This.vImgY, "Int", This.vImgW, "Int", This.vImgH, "Ptr", hDCCompat, "Int", 0, "Int", 0, "Int", This.actualVImgW, "Int", This.actualVImgH, "UInt", SRCCOPY))
						msgbox, 8208, PaintDC, BitBlt Failed!
					}
					else
					{
						if (!DllCall("gdi32\BitBlt", "Ptr", This.hDCWin, "Int", This.vImgX, "Int", This.vImgY, "Int", This.vImgW, "Int", This.vImgH, "Ptr", hDCCompat, "Int", 0, "Int", 0, "UInt", SRCCOPY))
						msgbox, 8208, PaintDC, BitBlt Failed!
					}

				This.SelectObject(hDCCompat, hBitmapOld, "Old Bitmap")
				}

				if (!(DllCall("gdi32\DeleteDC", "Ptr", hDCCompat)))
				msgbox, 8208, Compat DC, DC could not be deleted!

			}
			case 1, 2: ;IMAGE_ICON := 1, IMAGE_CURSOR := 1
			{
			DllCall("user32\DrawIconEx", "Ptr", This.hDCWin, "Int", This.vImgX, "Int", This.vImgY, "Ptr", This.hIcon, "Int", This.vImgW, "Int", This.vImgH, "UInt", 0, "Ptr", 0, "UInt", 0x3) ;DI_NORMAL := 0x3
				/*
				; DllCall("gdi32\DestroyIcon", "Ptr", This.hIcon) fails for AHK executable
				; AHK LoadImage does not use LR_SHARED
				; Consider above only if creating or copying an icon- whereas the following is ignored
				if (!(DllCall("gdi32\DeleteObject", "Ptr", This.hIcon)))
				msgbox, 8208, Icon Handle, Handle could not be deleted!
				*/
			}
		}
		This.releaseDC(This.hWndSaved[This.instance], This.hDCWin)
	}

	DrawBackground()
	{
		; for custom see  https://docs.microsoft.com/en-us/windows/win32/gdi/drawing-a-custom-window-background
		DllCall("gdi32\ExcludeClipRect", "Ptr", This.hDCWin, "Int", This.vImgX, "Int", This.vImgY, "Int", This.vImgX+This.vImgW, "Int", This.vImgY+This.vImgH)

		;SelectClipRgn not required
		; one pixel region
		hRgn := []
		hRgn := DllCall("gdi32\CreateRectRgn", "Int", 0, "Int", 0, "Int", 1, "Int", 1, "Ptr")
		; Updates hRgn to define the clipping region in This.hDCWin: turns out to be everything except the margins.
		DllCall("gdi32\GetClipRgn", "Ptr", This.hDCWin, "Ptr", hRgn)
		hBrush := DllCall("user32\GetSysColorBrush", "Int", 15, "Ptr") ;COLOR_BTNFACE := 15
		DllCall("gdi32\FillRgn", "Ptr", This.hDCWin, "Ptr", hRgn, "Ptr", hBrush)
		This.deleteObject(hRgn)
	}	

	ProcImgWHVal(value, height := 0)
	{
	retval := 0
		if (height)
		{
		dim := This.vImgH
		screenDim := A_ScreenHeight
		actualDim := This.actualVImgH
		}
		else
		{
		dim := This.vImgW
		screenDim := A_ScreenWidth
		actualDim := This.actualVImgW
		}

		if value is number
		{
			if (value > 10)
			{
			oldDim := dim
			retval := Floor(value)
			}
			else
			{

				if (value > 0)
				{
				oldDim := dim
				retval := Floor(value * actualDim)
				}
				else
				{
					if (value < 0 && value > -10)
					{
					oldDim := dim
					retval := -Floor(value * screenDim)
					}
					else
					retval := 0
				}
			}
		}

		if (height)
		This.oldVImgH := oldDim
		else
		This.oldVImgW := oldDim

	return retVal
	}


	GetPosProc(splashyInst, currVPos, init)
	{
		if (init)
		{
		; Init only! Position is never preserved, so rely on GuiGetPos
			if (This.vPosX == "c")
			This.vPosX := ""
			if (This.vPosY == "c")
			This.vPosY := ""
		}
		else
		{
		pointGet := This.GuiGetPos(This.hWnd(), 1)

			if (currVPos.x == "")
			{
			; arguably faster than type check
			if (This.vPosX == "" || This.vPosX == "l" || This.vPosX == "c"|| This.vPosX == "zero")
				currVPos.x := pointGet.x
				else
				currVPos.x := This.vPosX
			}

			if (currVPos.y == "")
			{
				if (This.vPosY == "" || This.vPosY == "l" || This.vPosY == "c" || This.vPosY == "zero")
				currVPos.y := pointGet.y
				else
				currVPos.y := This.vPosY
			}

		pointGet := ""
		}
	return currVPos
	}


	GuiGetPos(thisHWnd, hWndPos := 0)
	{
	static HWND_DESKTOP := 0, parentStat := 0

		if (hWndPos)
		{
			if (This.parent)
			parentHWnd := This.parentHWnd
			else
			parentHWnd := HWND_DESKTOP
		}

	VarSetCapacity(rect, 16, 0)

		if (DllCall("GetWindowRect", "Ptr", thisHWnd, "Ptr", &rect))
		{

		x := NumGet(rect, 0, "int")
		y := NumGet(rect, 4, "int")

			if (!hWndPos)
			{
			w := NumGet(rect, 8, "int")
			w := w - x

			h := NumGet(rect, 12, "int")
			h := h - y

			VarSetCapacity(rect, 0)
			return {w: w, h: h}
			}

		VarSetCapacity(point, 8, 0)

		NumPut(x, point, 0, "Int"), NumPut(y, point, 4, "Int")

			if (parentHWnd)
			{
				if (!DllCall("user32\ScreenToClient", "Ptr", parentHWnd, "Ptr", &point, "int"))
				return 0

				;if (!(DllCall("User32.dll\MapWindowPoints", "Ptr", HWND_DESKTOP, "Ptr", parentHWnd, "Ptr", &point, "UInt", 1)))
				;return 0
			}
			else
			{
				if (parentStat != parentHWnd)
				{
				if !DllCall("user32\ClientToScreen", "Ptr", parentStat, "Ptr", &point, "int")
				return 0
				}
			}
			
		x := NumGet(point, 0, "Int"), y := NumGet(point, 4, "Int")

		VarSetCapacity(point, 0)
		}
		else
		return 0

	VarSetCapacity(rect, 0)
	parentStat := parentHWnd

	return {x: x, y: y}
	}


	TransPosVal(vPos, parentDim, winDim)
	{
		if (vPos == "c")
		{
			if (winDim < parentDim)
			vPos := (parentDim - winDim)/2
			else
			vPos := 0
		}
		else
		{
			if (vPos == "zero")
			vPos := 0
			else
			if (vPos == "l")
			vPos := ""
		}
	return vPos
	}

	GetPosVal(vPosX, vPosY, currVPos, parentDimW, parentDimH, winDimW, winDimH, parentHWnd)
	{

	vPosXIn := vPosX
	vPosYIn := vPosY
	vPosX := This.TransPosVal(vPosX, parentDimW, winDimW)
	vPosY := This.TransPosVal(vPosY, parentDimH, winDimH)

		if (vPosX == "")
		{
			if (vPosY == "")
			return {x: currVPos.x, y: currVPos.y}
			else
			vPosXNew := (currVPos.x)?currVPos.x:0
		}
		else
		vPosXNew := vPosX

		if (vPosY == "")
		vPosYNew := (currVPos.y)?currVPos.y:0
		else
		vPosYNew := vPosY
	
		if (parentHWnd)
		{
		VarSetCapacity(point, 8, 0)
		NumPut(vPosXNew, point, 0, "Int")
		NumPut(vPosYNew, point, 4, "Int")

			if !DllCall("user32\ScreenToClient", "Ptr", parentHWnd, "Ptr", &point, "int")
			return 0

			if (vPosXIn == "c")
			{
			parentPoint := This.GuiGetPos(parentHWnd, 1)
			vPosXNew := vPosX + parentPoint.x
			}
			else
			vPosXNew := NumGet(point, 0, "Int")

			if (vPosYIn == "c")
			{
				if (parentPoint.x == "")
				parentPoint := This.GuiGetPos(parentHWnd, 1)
			vPosYNew := vPosY + parentPoint.y
			}
			else
			vPosYNew := NumGet(point, 4, "Int")

		VarSetCapacity(point, 0)

			if (vPosXNew < 0)
			vPosXNew := 0
			else
			{
				if (vPosXNew > parentDimW)
				vPosXNew := parentDimW - winDimW
			}
			if (vPosYNew < 0)
			vPosYNew := 0
			else
			{
				if (vPosYNew > parentDimH)
				vPosYNew := parentDimH - winDimH
			}
		}

	return {x: (vPosX == "")? currVPos.x: vPosXNew, y: (vPosY == "")? currVPos.y : vPosYNew}

	}


	DoText(splashyInst, hWnd, text, ByRef currVPos, ByRef currSplashyInstW, currSplashyInstH, init, sub := 0)
	{
	static SS_Center := 0X1, SWP_SHOWWINDOW := 0x0040, mainTextSize := [], subTextSize := []
	init := 0
		if (StrLen(text))
		{
		; Note default font styles for main & sub differ
			if (sub)
			Gui, %splashyInst%: Font, % "norm s" . This.subFontSize . " w" . This.subFontWeight . " q" . This.subFontQuality . This.subFontItalic . This.subFontStrike . This.subFontUnderline, % This.subFontName
			else
			Gui, %splashyInst%: Font, % "norm s" . This.mainFontSize . " w" . This.mainFontWeight . " q" . This.mainFontQuality . This.mainFontItalic . This.mainFontStrike . This.mainFontUnderline, % This.mainFontName

			if (hWnd)
			{
			GuiControl, %splashyInst%: Text, %hWnd%, % text
			GuiControl, %splashyInst%: Font, %hWnd%
			}
			else
			{
			init := 1
			Gui, %splashyInst%: Add, Text, % "X0 W" . This.vImgW . " Y" . (sub?currSplashyInstH:This.vMgnY) . " HWND" . "hWnd", % text
			}

			if (sub)
			{
			This.subTextHWnd[This.instance] := hWnd
			subTextSize := This.Text_Dims(text, hWnd)
			}
			else
			{
			This.mainTextHWnd[This.instance] := hWnd
			mainTextSize := This.Text_Dims(text, hWnd)
			}

			if (This.vImgTxtSize)
			{
				; Not so precise- otherwise very fiddly
				if (sub)
				{
					if (!(This.mainTextHWnd[This.instance] && mainTextSize[1] > subTextSize[1]))
					{
					currSplashyInstW += subTextSize[1] - This.vImgW
					This.vImgW := subTextSize[1]
					This.inputVImgW := ""
					}
				}
				else
				{
					if (!(This.subTextHWnd[This.instance] && subTextSize[1] > mainTextSize[1]))
					{
					currSplashyInstW += mainTextSize[1] - This.vImgW
					This.vImgW := mainTextSize[1]
					This.inputVImgW := ""
					}
				}
			}

			if (This.Parent)
			{
			; vMgnx, vMgnY not applicable here
 			This.Setparent(1, !sub?hWnd:0, sub?hWnd:0)

			currVPos := This.GetPosVal(This.vPosX, This.vPosY, currVPos, A_ScreenWidth, A_ScreenHeight, currSplashyInstW, currSplashyInstH, This.parentHWnd)
			; Init only! Position is never preserved, so rely on GuiGetPos
			if (init)
			currVPos := This.GetPosProc(splashyInst, currVPos, 1)

;			;Margins not required!
			WinSet, Style, +%SS_Center%, ahk_id %hWnd%
			WinMove ahk_id %hWnd%, , % currVPos.x, % currVPos.y + (sub?This.vImgH:0), % This.vImgW, % sub?subTextSize[2]:mainTextSize[2]
			;DllCall("SetWindowPos", "UInt", hWnd, "UInt", 0, "Int", This.currVPos.x, "Int", This.currVPos.y, "Int", This.vImgW, "Int", mainTextSize[2], "UInt", 0x0004)


			WinSet, AlwaysOnTop, 1, ahk_id %hWnd%
			WinShow, ahk_id %hWnd%
			}
			else
			{
			; Remove and set the style first- Done so the margins can be centred.
			WinSet, Style, -%SS_Center%, ahk_id %hWnd%
			DllCall("SetWindowPos", "Ptr", hWnd, "Ptr", 0, "Int", 0, "Int", 0,"Int", 0,"Int", 0,"UInt", SWP_SHOWWINDOW)

				if (sub)
				{
				This.Setparent(0, , hWnd)
				spr := This.vImgW - subTextSize[1] + 2 * This.vMgnX
				spr := (spr > 0)?((This.vImgTxtSize)? 0: spr)/2: 0
				GuiControl, %splashyInst%: Move, %hWnd%, % "X" . spr . " Y" . currSplashyInstH . " W" . This.vImgW . " H" . subTextSize[2]
				}
				else
				{
				This.Setparent(0, hWnd)
				spr := This.vImgW - mainTextSize[1] + 2 * This.vMgnX
				spr := (spr > 0)?((This.vImgTxtSize)? 0: spr/2): 0
				GuiControl, %splashyInst%: Move, %hWnd%, % "X" . spr . " Y" . This.vMgnY . " W" . This.vImgW . " H" . mainTextSize[2]
				}
			GuiControl, %splashyInst%: Show, %hWnd% ; in case of previously hidden
			}

			GuiControl, %splashyInst%: Font, %hWnd%

			

			;ControlSetText, , %mainText%, % "ahk_id" . hWnd
			; This sends more paint messages to parent
			;ControlMove, , % This.vMgnX, % This.vMgnY, This.vImgW , Text_Dims(mainText, hWnd), % "ahk_id" . hWnd

			if (This.transCol)
			(sub)? This.subBkgdColour := This.ValidateColour(This.bkgdColour, 1): This.mainBkgdColour := This.ValidateColour(This.bkgdColour, 1)

		This.SubClassTextCtl(hWnd)
		return % (sub)?subTextSize[2]:mainTextSize[2]
		}
		else
		{
			if (hWnd)
			{
			GuiControl, %splashyInst%: Hide, %hWnd%

				if (sub)
				subTextSize := ""
				else
				mainTextSize := ""
			}

		return 0
		}
	}

	SetParent(parentSetStatus, mainHWndIn := "", subHWndIn := "")
	{
	Static lastParentStatus := 0, mainHWnd := 0, subHWnd := 0

		if (lastParentStatus == parentSetStatus)
		return

		if (mainHWndIn)
		hWnd := mainHWnd := mainHWndIn
		else
		{
			if (subHWndIn)
			hWnd := subHWnd := subHWndIn
			else
			{
				if (mainHWndIn == 0)
				hWnd := mainHWnd
				else
				hWnd := subHWnd
			}
		}

		if (parentSetStatus)
		{
			if (DllCall("SetParent", "Ptr", hWnd, "Ptr", This.parentHWnd) != This.hWnd())
			msgbox, 8192, SetParent, Cannot set parent for control!
		}
		else
		{
			if (DllCall("SetParent", "Ptr", hWnd, "Ptr", This.hWnd()) != This.parentHWnd)
			msgbox, 8192, SetParent, Cannot set parent for control!
		}
	lastParentStatus := parentSetStatus
	}

	SetParentFlag()
	{
	Static WS_CLIPCHILDREN := 0x02000000
	DetectHiddenWindows, On

	spr := WinExist()
	; AutoHotkeyGui when SplashyTest launches this from a gui thread
	WinGet, WindowList, List , ahk_class AutoHotkeyGUI

	DetectHiddenWindows, Off

		Loop %WindowList%
		{
			if (spr == WindowList%A_Index%)
			{
			; Get clip style
			Winget, spr1, Style, ahk_id %spr%
				if (spr1 & WS_CLIPCHILDREN)
				This.parentClip := WS_CLIPCHILDREN
				else
				This.parentClip := 0
			return spr
			}
		}
	return "Error"
	}

	Text_Dims(Text, hWnd)
	{
	Static WM_GETFONT := 0x0031
	FontSize := [], hDCScreen := 0, outSize := [0, 0]
	;https://www.autohotkey.com/boards/viewtopic.php?f=76&t=9130&p=50713#p50713

	StrReplace(Text, "`r`n", "`r`n", spr1)
	StrReplace(Text, "`n", "`n", spr2)
	spr1 += spr2 + 1

	spr2 := "" ; get longest of multiline
		loop, Parse, Text, `n, `r
		{
		if (StrLen(A_Loopfield)) > StrLen(spr2)
		spr2 := A_Loopfield
		}

	HFONT := DllCall("User32.dll\SendMessage", "Ptr", hWnd, "Int", WM_GETFONT, "Ptr", 0, "Ptr", 0)

	hDCScreen := DllCall("user32\GetDC", "Ptr", 0, "Ptr")

		if (HFONT_OLD := This.SelectObject(hDCScreen, HFONT, "Font"))
		{

		VarSetCapacity(FontSize, 8)
		DllCall("GetTextExtentPoint32", "UPtr", hDCScreen, "Str", spr2, "Int", StrLen(spr2), "UPtr", &FontSize)
		outSize[1] := NumGet(FontSize, 0, "UInt")
		DllCall("GetTextExtentPoint32", "UPtr", hDCScreen, "Str", Text, "Int", StrLen(Text), "UPtr", &FontSize)
		outSize[2] := NumGet(FontSize, 4, "UInt") * spr1

		; clean up

		This.SelectObject(hDCScreen, HFONT_OLD, "Old Font")
		; If not created, DeleteObject NOT required for This HFONT

		This.releaseDC(0, hDCScreen)
		VarSetCapacity(FontSize, 0)
		return outSize
		}
		else
		{
		This.releaseDC(0, hDCScreen)
		return 0
		}
	}

	B64Decode(B64, nBytes := "", W := "", H := "")
	{
	Bin = {}, BLen := 0, hICON := 0  

		if !nBytes
		nBytes := ceil(StrLen(StrReplace( B64, "=", "=", e))/4*3) - e

	VarSetCapacity( Bin, nBytes, 0 ), BLen := StrLen(B64)
		If DllCall( "Crypt32.dll\CryptStringToBinary", "Ptr", &B64, "UInt", BLen, "UInt", 0x1
		, "Ptr", &Bin, "UInt*", nBytes, "Int", 0, "Int", 0)
		hICON := DllCall( "CreateIconFromResourceEx", "Ptr", &Bin, "UInt", nBytes, "Int", True
		, "UInt", "0x30000", "Int", W, "Int", H, "UInt", 0, "UPtr")
		; 0X30000: version number of the icon or cursor format for the resource bits pointed to by the pbIconBits 
	Return hICON
	}

	vMovable
	{
		set
		{
		This._vMovable := value
		}
		get
		{
		return This._vMovable
		}
	}

	hWnd()
	{
		if (!(spr := This.hWndSaved[This.instance]))
		{
		spr := "Splashy" . (This.instance)
		Gui, %spr%: +HWNDspr
		This.hWndSaved[This.instance] := spr
		}
	return spr
	}

	SelectObject(hDC, hgdiobj, type)
	{
	static HGDI_ERROR := 0xFFFFFFFF

	hRet := DllCall("Gdi32.dll\SelectObject", "Ptr", hDC, "Ptr", hgdiobj, "Ptr")

		if (!hRet || hRet == HGDI_ERROR)
		{
		msgbox, 8208, GDI Object, % "Selection failed for " type "`nError code is: " . ((hRet == HGDI_ERROR)? "HGDI_ERROR: ": "Unknown: ") . "The errorLevel is " ErrorLevel ": " . A_LastError
		return 0
		}
		else
		return hRet
	}

	deleteObject(hDC, hgdiobj)
	{
		if !DllCall("Gdi32.dll\DeleteObject", "Ptr", hObject)
		msgbox, 8208, GDI Object, % "Deletion failed `nError code is: " . "ErrorLevel " ErrorLevel ": " . A_LastError
	}
		releaseDC(hWnd, hDC)
	{
		if !DllCall("ReleaseDC", "Ptr", hWnd, "UPtr", hDC)
		msgbox, 8208, Device Context, % "Release failed `nError code is: " . "ErrorLevel " ErrorLevel ": " . A_LastError
	}

	SaveRestoreUserParms(Restore := 0)
	{
	Static userWorkingDir := "", userStringCaseSense := "", userDHW := ""

		if (Restore)
		{
			SetWorkingDir %userWorkingDir%
			StringCaseSense, %userStringCaseSense%
			DetectHiddenWindows %userDHW%
		}
		else
		{
			userDHW := A_DetectHiddenWindows
			userWorkingDir := A_WorkingDir
			userStringCaseSense := A_StringCaseSense
		}
	}

	Destroy()
	{
	SetWorkingDir %A_ScriptDir%
		for key, value in % This.downloadedPathNames
		{
			if (FileExist(value))
			FileDelete, % value
		}

	This.SaveRestoreUserParms(1)

	if (not This.vImgType)  ; IMAGE_BITMAP (0) or the ImageType parameter was omitted.
	DllCall("DeleteObject", "Ptr", This.hBitmap)
	else if (This.vImgType == 1)  ; IMAGE_ICON
	DllCall("DestroyIcon", "Ptr", This.hIcon)
	else if (This.vImgType == 2)  ; IMAGE_CURSOR
	DllCall("DestroyCursor", "Ptr", This.hIcon)

	This.SetCapacity(downloadedPathNames, 0)
	This.SetCapacity(downloadedUrlNames, 0)
		for key in This.hWndSaved
		{
		value := "Splashy" . key
		Gui, %value%: Destroy
		Splashy.NewWndProc.clbk[key] := "" ; should invoke __Delete
		This.hWndSaved[key] := 0
		This.mainTextHWnd[key] := 0
		This.subTextHWnd[key] := 0
		}
	This.updateFlag := 0
	This.NewWndObj := ""
	This.SubClassTextCtl(0, 1)

	if (This.pToken)
	DllCall("GdiPlus.dll\GdiplusShutdown", "Ptr", This.pToken)
	if (This.hGDIPLUS)
	DllCall("FreeLibrary", "Ptr", This.hGDIPLUS)

	;This.Delete("", chr(255))
	This.SetCapacity(0)
	This.base := ""
	}

	DeleteHandles()
	{
		if (This.hBitmap)
		{
		DllCall("DeleteObject", "Ptr", This.hBitmap)
		This.hBitmap := 0
		}
		else
		{
			if (This.hIcon)
			{
			DllCall("DestroyIcon", "Ptr", This.hIcon)
			This.hIcon := 0
			}
		}
	}
	; ##################################################################################
}
;=====================================================================================