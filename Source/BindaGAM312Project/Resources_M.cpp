// Fill out your copyright notice in the Description page of Project Settings.


#include "Resources_M.h"

// Sets default values
AResources_M::AResources_M()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Sets up the components of the actor
	ResourceNameTxt = CreateDefaultSubobject<UTextRenderComponent>(TEXT("Text Render"));
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));

	RootComponent = Mesh;

	ResourceNameTxt->SetupAttachment(Mesh);

}

// Called when the game starts or when spawned
void AResources_M::BeginPlay()
{
	Super::BeginPlay();

	// Sets the text on the resource actor to the name of the resource, since it is blank by default
	tempText = tempText.FromString(resourceName);

	ResourceNameTxt->SetText(tempText);
	
}

// Called every frame
void AResources_M::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

