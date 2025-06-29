// Fill out your copyright notice in the Description page of Project Settings.


#include "Types/TagTypes.h"


namespace MovementModeTags
{
	UE_DEFINE_GAMEPLAY_TAG(Grounded, FName{TEXTVIEW("GASP.MovementMode.OnGrounded")});
	UE_DEFINE_GAMEPLAY_TAG(InAir, FName{TEXTVIEW("GASP.MovementMode.InAir")});
}

namespace OverlayModeTags
{
	UE_DEFINE_GAMEPLAY_TAG(Default, FName{TEXTVIEW("GASP.OverlayMode.Default")})
	UE_DEFINE_GAMEPLAY_TAG(Masculine, FName{TEXTVIEW("GASP.OverlayMode.Masculine")})
	UE_DEFINE_GAMEPLAY_TAG(Feminine, FName{TEXTVIEW("GASP.OverlayMode.Feminine")})
	UE_DEFINE_GAMEPLAY_TAG(Injured, FName{TEXTVIEW("GASP.OverlayMode.Injured")})
	UE_DEFINE_GAMEPLAY_TAG(HandsTied, FName{TEXTVIEW("GASP.OverlayMode.HandsTied")})
	UE_DEFINE_GAMEPLAY_TAG(Rifle, FName{TEXTVIEW("GASP.OverlayMode.Rifle")})
	UE_DEFINE_GAMEPLAY_TAG(PistolOneHanded, FName{TEXTVIEW("GASP.OverlayMode.PistolOneHanded")})
	UE_DEFINE_GAMEPLAY_TAG(PistolTwoHanded, FName{TEXTVIEW("GASP.OverlayMode.PistolTwoHanded")})
	UE_DEFINE_GAMEPLAY_TAG(Bow, FName{TEXTVIEW("GASP.OverlayMode.Bow")})
	UE_DEFINE_GAMEPLAY_TAG(Torch, FName{TEXTVIEW("GASP.OverlayMode.Torch")})
	UE_DEFINE_GAMEPLAY_TAG(Binoculars, FName{TEXTVIEW("GASP.OverlayMode.Binoculars")})
	UE_DEFINE_GAMEPLAY_TAG(Box, FName{TEXTVIEW("GASP.OverlayMode.Box")})
	UE_DEFINE_GAMEPLAY_TAG(Barrel, FName{TEXTVIEW("GASP.OverlayMode.Barrel")})
}

namespace LocomotionActionTags
{
	UE_DEFINE_GAMEPLAY_TAG(Ragdoll, FName{TEXTVIEW("GASP.LocomotionAction.Ragdoll")})
	UE_DEFINE_GAMEPLAY_TAG(Vault, FName{TEXTVIEW("GASP.LocomotionAction.Vault")})
	UE_DEFINE_GAMEPLAY_TAG(Mantle, FName{TEXTVIEW("GASP.LocomotionAction.Mantle")})
	UE_DEFINE_GAMEPLAY_TAG(Hurdle, FName{TEXTVIEW("GASP.LocomotionAction.Hurdle")})
}

namespace FoleyTags
{
	UE_DEFINE_GAMEPLAY_TAG(Handplant, FName{TEXTVIEW("Foley.Event.Handplant")})
	UE_DEFINE_GAMEPLAY_TAG(Run, FName{TEXTVIEW("Foley.Event.Run")})
	UE_DEFINE_GAMEPLAY_TAG(RunBackwds, FName{TEXTVIEW("Foley.Event.RunBackwds")})
	UE_DEFINE_GAMEPLAY_TAG(RunStrafe, FName{TEXTVIEW("Foley.Event.RunStrafe")})
	UE_DEFINE_GAMEPLAY_TAG(Scuff, FName{TEXTVIEW("Foley.Event.Scuff")})
	UE_DEFINE_GAMEPLAY_TAG(ScuffPivot, FName{TEXTVIEW("Foley.Event.ScuffPivot")})
	UE_DEFINE_GAMEPLAY_TAG(Walk, FName{TEXTVIEW("Foley.Event.Walk")})
	UE_DEFINE_GAMEPLAY_TAG(WalkBackwds, FName{TEXTVIEW("Foley.Event.WalkBackwds")})
	UE_DEFINE_GAMEPLAY_TAG(Jump, FName{TEXTVIEW("Foley.Event.Jump")})
	UE_DEFINE_GAMEPLAY_TAG(Land, FName{TEXTVIEW("Foley.Event.Land")})
	UE_DEFINE_GAMEPLAY_TAG(ScuffWall, FName{TEXTVIEW("Foley.Event.ScuffWall")})
	UE_DEFINE_GAMEPLAY_TAG(Tumble, FName{TEXTVIEW("Foley.Event.Tumble")})
}


namespace StanceTags
{
	UE_DEFINE_GAMEPLAY_TAG(Standing, FName{TEXTVIEW("GASP.Stance.Standing")})
	UE_DEFINE_GAMEPLAY_TAG(Crouching, FName{TEXTVIEW("GASP.Stance.Crouching")})
}

namespace MovementStateTags
{
	UE_DEFINE_GAMEPLAY_TAG(Moving, FName{TEXTVIEW("GASP.MovementState.Moving")});
	UE_DEFINE_GAMEPLAY_TAG(Idle, FName{TEXTVIEW("GASP.MovementState.Idle")});
}
