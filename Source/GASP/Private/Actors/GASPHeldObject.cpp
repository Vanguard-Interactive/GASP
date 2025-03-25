// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/GASPHeldObject.h"


// Sets default values
AGASPHeldObject::AGASPHeldObject()
{
	bReplicates = true;
	bNetUseOwnerRelevancy = true;
	
	MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;
}

