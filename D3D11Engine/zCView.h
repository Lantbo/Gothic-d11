#pragma once
#include "pch.h"
#include "zTypes.h"
#include "HookedFunctions.h"
#include "oCGame.h"
#include "zViewTypes.h"


class zCViewDraw {
public:
    static void Hook() {
        //XHook( HookedFunctions::OriginalFunctions.original_zCViewDrawGetScreen, GothicMemoryLocations::zCViewDraw::GetScreen, GetScreen );
    }
    
    static zCViewDraw& GetScreen() {
        return reinterpret_cast<zCViewDraw&( __fastcall* )( void )>( GothicMemoryLocations::zCViewDraw::GetScreen )();
    }

    void __fastcall SetVirtualSize(POINT& pt) {
        reinterpret_cast<void( __fastcall* )( zCViewDraw*, POINT& )>( GothicMemoryLocations::zCViewDraw::SetVirtualSize )( this, pt );
    }
};

class zCView {
public:

    /** Hooks the functions of this Class */
    static void Hook() {
        zCViewDraw::Hook();
        DWORD dwProtect;

        VirtualProtect( (void*)GothicMemoryLocations::zCView::SetMode, 0x1B9, PAGE_EXECUTE_READWRITE, &dwProtect ); // zCView::SetMode

        // Replace the actual mode-change in zCView::SetVirtualMode. Only do the UI-Changes.
        REPLACE_RANGE( GothicMemoryLocations::zCView::REPL_SetMode_ModechangeStart, GothicMemoryLocations::zCView::REPL_SetMode_ModechangeEnd - 1, INST_NOP );

#if BUILD_GOTHIC_2_6_fix
        // .text:007A62A0; void __thiscall zCView::BlitText(zCView * __hidden this)
        XHook( HookedFunctions::OriginalFunctions.original_zCViewBlitText, GothicMemoryLocations::zCView::BlitText, hooked_BlitText );
        XHook( HookedFunctions::OriginalFunctions.original_zCViewPrint, GothicMemoryLocations::zCView::Print, hooked_Print );
        //XHook( HookedFunctions::OriginalFunctions.original_zCViewBlit, GothicMemoryLocations::zCView::Blit, hooked_Blit );
#endif
    }

    static void SetWindowMode( int x, int y, int bpp ) {
        reinterpret_cast<void( __fastcall* )( DWORD, DWORD, int, int, int, void* )>
            ( GothicMemoryLocations::zCView::Vid_SetMode )( *reinterpret_cast<DWORD*>(GothicMemoryLocations::GlobalObjects::zRenderer), 0, x, y, bpp, nullptr );
    }

    static void SetVirtualMode( int x, int y, int bpp ) {
        reinterpret_cast<void(__cdecl*)( int, int, int, void* )>
            ( GothicMemoryLocations::zCView::SetMode )( x, y, bpp, nullptr );
    }

#if BUILD_GOTHIC_2_6_fix
    /*
    static void __fastcall hooked_Blit(_zCView* thisptr, void* unknwn) {

        if (true || !Engine::GAPI->GetRendererState().RendererSettings.EnableCustomFontRendering) {
            HookedFunctions::OriginalFunctions.original_zCViewBlit(thisptr);
            return;
        }

        if (thisptr->viewID == 1 /* VIEW_VIEWPORT *\/) return;
        if (thisptr == GetScreen()) return;
        hook_infunc
            auto oldDepthState = Engine::GAPI->GetRendererState().DepthState.Clone();

            if (!thisptr->m_bFillZ) Engine::GAPI->GetRendererState().DepthState.DepthWriteEnabled = false;
            else Engine::GAPI->GetRendererState().DepthState.DepthWriteEnabled = true;

            Engine::GAPI->GetRendererState().DepthState.DepthBufferCompareFunc = GothicDepthBufferStateInfo::CF_COMPARISON_ALWAYS;
            Engine::GAPI->GetRendererState().DepthState.SetDirty();

            Engine::GraphicsEngine->UpdateRenderStates();

            HookedFunctions::OriginalFunctions.original_zCViewBlit(thisptr);

            oldDepthState.ApplyTo(Engine::GAPI->GetRendererState().DepthState);
            Engine::GraphicsEngine->UpdateRenderStates();
        hook_outfunc
    }
    */

    static void __fastcall hooked_Print( _zCView* thisptr, void* unknwn, int x, int y, const zSTRING& s ) {
        if ( !Engine::GAPI->GetRendererState().RendererSettings.EnableCustomFontRendering ) {
            HookedFunctions::OriginalFunctions.original_zCViewPrint( thisptr, x, y, s );
            return;
        }
        if ( !thisptr->font ) return;
        thisptr->scrollTimer = 0;

        // Instantly blit Viewport/global-screen
        if ( (thisptr->viewID == 1)
            || (thisptr == GetScreen()) ) {
            Engine::GraphicsEngine->DrawString(
                s.ToChar(),
                static_cast<float>(thisptr->pposx + thisptr->nax( x )),
                static_cast<float>(thisptr->pposy + thisptr->nay( y )),
                thisptr->font, thisptr->fontColor );
        } else {
            // create a textview for later blitting
            thisptr->CreateText( x, y, s );
        }
    }

    static void __fastcall hooked_BlitText( _zCView* thisptr, void* unknwn ) {
        if ( !Engine::GAPI->GetRendererState().RendererSettings.EnableCustomFontRendering ) {
            HookedFunctions::OriginalFunctions.original_zCViewBlitText( thisptr );
            return;
        }

        thisptr->CheckAutoScroll();
        thisptr->CheckTimedText();

        if ( !thisptr->isOpen ) return;

        float x, y;

        zCList <zCViewText>* textNode = thisptr->textLines.next;
        zCViewText* text = nullptr;
        zColor fontColor = thisptr->fontColor;
        while ( textNode ) {

            text = textNode->data;
            textNode = textNode->next;

            if ( text->colored ) { fontColor = text->color; }
            //else                 { fontColor = thisptr->fontColor;}

            x = static_cast<float>(thisptr->pposx + thisptr->nax( text->posx ));
            y = static_cast<float>(thisptr->pposy + thisptr->nay( text->posy ));

            if ( !thisptr->font ) continue;

            Engine::GraphicsEngine->DrawString( text->text.ToChar(), x, y, text->font, fontColor );
        }
    }
    static _zCView* GetScreen() { return *(_zCView**)GothicMemoryLocations::GlobalObjects::screen; }

#endif

    /** Prints a message to the screen */
    void PrintTimed( int posX, int posY, const zSTRING& strMessage, float time = 3000.0f, DWORD* col = nullptr ) {
        reinterpret_cast<void( __fastcall* )( zCView*, int, int, int, const zSTRING&, float, DWORD* )>
            ( GothicMemoryLocations::zCView::PrintTimed )(this, 0, posX, posY, strMessage, time, col);
    }
};
