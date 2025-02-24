// Fill out your copyright notice in the Description page of Project Settings.


#include "Types/GameplayTags.h"


namespace OverlayModeTags
{
	UE_DEFINE_GAMEPLAY_TAG(Default, FName{TEXT("GASP.OverlayMode.Default")})
	UE_DEFINE_GAMEPLAY_TAG(Masculine, FName{TEXT("GASP.OverlayMode.Masculine")})
	UE_DEFINE_GAMEPLAY_TAG(Feminine, FName{TEXT("GASP.OverlayMode.Feminine")})
	UE_DEFINE_GAMEPLAY_TAG(Injured, FName{TEXT("GASP.OverlayMode.Injured")})
	UE_DEFINE_GAMEPLAY_TAG(HandsTied, FName{TEXT("GASP.OverlayMode.HandsTied")})
	UE_DEFINE_GAMEPLAY_TAG(Rifle, FName{TEXT("GASP.OverlayMode.Rifle")})
	UE_DEFINE_GAMEPLAY_TAG(PistolOneHanded, FName{TEXT("GASP.OverlayMode.PistolOneHanded")})
	UE_DEFINE_GAMEPLAY_TAG(PistolTwoHanded, FName{TEXT("GASP.OverlayMode.PistolTwoHanded")})
	UE_DEFINE_GAMEPLAY_TAG(Bow, FName{TEXT("GASP.OverlayMode.Bow")})
	UE_DEFINE_GAMEPLAY_TAG(Torch, FName{TEXT("GASP.OverlayMode.Torch")})
	UE_DEFINE_GAMEPLAY_TAG(Binoculars, FName{TEXT("GASP.OverlayMode.Binoculars")})
	UE_DEFINE_GAMEPLAY_TAG(Box, FName{TEXT("GASP.OverlayMode.Box")})
	UE_DEFINE_GAMEPLAY_TAG(Barrel, FName{TEXT("GASP.OverlayMode.Barrel")})
}

namespace LocomotionActionTags
{
	UE_DEFINE_GAMEPLAY_TAG(None, FName{TEXT("GASP.LocomotionAction.None")})
	UE_DEFINE_GAMEPLAY_TAG(Ragdoll, FName{TEXT("GASP.LocomotionAction.Ragdoll")})
	UE_DEFINE_GAMEPLAY_TAG(Vault, FName{TEXT("GASP.LocomotionAction.Vault")})
	UE_DEFINE_GAMEPLAY_TAG(Mantle, FName{TEXT("GASP.LocomotionAction.Mantle")})
	UE_DEFINE_GAMEPLAY_TAG(Hurdle, FName{TEXT("GASP.LocomotionAction.Hurdle")})
}

namespace FoleyTags
{
	UE_DEFINE_GAMEPLAY_TAG(Handplant, FName{TEXT("Foley.Event.Handplant")})
	UE_DEFINE_GAMEPLAY_TAG(Run, FName{TEXT("Foley.Event.Run")})
	UE_DEFINE_GAMEPLAY_TAG(RunBackwds, FName{TEXT("Foley.Event.RunBackwds")})
	UE_DEFINE_GAMEPLAY_TAG(RunStrafe, FName{TEXT("Foley.Event.RunStrafe")})
	UE_DEFINE_GAMEPLAY_TAG(Scuff, FName{TEXT("Foley.Event.Scuff")})
	UE_DEFINE_GAMEPLAY_TAG(ScuffPivot, FName{TEXT("Foley.Event.ScuffPivot")})
	UE_DEFINE_GAMEPLAY_TAG(Walk, FName{TEXT("Foley.Event.Walk")})
	UE_DEFINE_GAMEPLAY_TAG(WalkBackwds, FName{TEXT("Foley.Event.WalkBackwds")})
	UE_DEFINE_GAMEPLAY_TAG(Jump, FName{TEXT("Foley.Event.Jump")})
	UE_DEFINE_GAMEPLAY_TAG(Land, FName{TEXT("Foley.Event.Land")})
	UE_DEFINE_GAMEPLAY_TAG(ScuffWall, FName{TEXT("Foley.Event.ScuffWall")})
	UE_DEFINE_GAMEPLAY_TAG(Tumble, FName{TEXT("Foley.Event.Tumble")})
}
