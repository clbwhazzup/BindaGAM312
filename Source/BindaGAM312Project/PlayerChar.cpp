// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerChar.h"

// Sets default values
APlayerChar::APlayerChar()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Initial camera component setup
	PlayerCamComp = CreateDefaultSubobject<UCameraComponent>(TEXT("First Person Cam"));

	// Attaches camera to character mesh in head slot
	PlayerCamComp->SetupAttachment(GetMesh(), "head");

	// Links camera rotation to controller
	PlayerCamComp->bUsePawnControlRotation = true;

	// Sets up building pieces array, resources array, and resources name array
	BuildingArray.SetNum(3);
	ResourcesArray.SetNum(3);
	ResourcesNameArray.Add(TEXT("Wood"));
	ResourcesNameArray.Add(TEXT("Stone"));
	ResourcesNameArray.Add(TEXT("Berry"));

}

// Called when the game starts or when spawned
void APlayerChar::BeginPlay()
{
	Super::BeginPlay();

	// Starts a timer that calls DecreaseStats every 2 seconds
	FTimerHandle StatsTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(StatsTimerHandle, this, &APlayerChar::DecreaseStats, 2.0f, true);

	if (objWidget)
	{
		objWidget->UpdatebuildObj(0.0f);
		objWidget->UpdatematObj(0.0f);
	}
}

// Called every frame
void APlayerChar::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Updates health, hunger, and stamina bars
	playerUI->UpdateBars(Health, Hunger, Stamina);
	
	// Constantly updates the location of the building part being placed, so the player knows where it will place
	if (isBuilding)
	{
		if (spawnedPart)
		{
			FVector StartLocation = PlayerCamComp->GetComponentLocation();
			FVector Direction = PlayerCamComp->GetForwardVector() * 400.0f;
			FVector EndLocation = StartLocation + Direction;
			spawnedPart->SetActorLocation(EndLocation);
		}
	}

}

// Called to bind functionality to input
void APlayerChar::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAxis("MoveForward", this, &APlayerChar::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &APlayerChar::MoveRight);
	PlayerInputComponent->BindAxis("LookUp", this, &APlayerChar::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Turn", this, &APlayerChar::AddControllerYawInput);
	PlayerInputComponent->BindAction("JumpEvent", IE_Pressed, this, &APlayerChar::StartJump);
	PlayerInputComponent->BindAction("JumpEvent", IE_Released, this, &APlayerChar::StopJump);

	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &APlayerChar::FindObject);
	PlayerInputComponent->BindAction("RotPart", IE_Pressed, this, &APlayerChar::RotateBuilding);

}

// Checks direction, and calls AddMovementInput to move character
void APlayerChar::MoveForward(float axisValue)
{
	FVector Direction = FRotationMatrix(Controller->GetControlRotation()).GetScaledAxis(EAxis::X);
	AddMovementInput(Direction, axisValue);
}

// Checks direction, and calls AddMovementInput to move character
void APlayerChar::MoveRight(float axisValue)
{
	FVector Direction = FRotationMatrix(Controller->GetControlRotation()).GetScaledAxis(EAxis::Y);
	AddMovementInput(Direction, axisValue);
}

// Uses built-in jump method by setting bPressedJump to true
void APlayerChar::StartJump()
{
	bPressedJump = true;
}

// Uses built-in jump method by setting bPressedJump to false
void APlayerChar::StopJump()
{
	bPressedJump = false;
}

// Line trace for interaction. Called by interact key bind
void APlayerChar::FindObject()
{
	// Sets up inputs for line trace
	FHitResult HitResult;
	FVector StartLocation = PlayerCamComp->GetComponentLocation();
	FVector Direction = PlayerCamComp->GetForwardVector() * 800.0f;
	FVector EndLocation = StartLocation + Direction;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	QueryParams.bTraceComplex = true;
	QueryParams.bReturnFaceIndex = true;

	// Checks if building is being placed before doing interaction line trace. Else, it sets isBuilding to false to place the piece. This stops the code block in Tick that updates the location of the piece
	if (!isBuilding)
	{
		// Checks if line trace hits an actor
		if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Visibility, QueryParams))
		{
			// Casting, sets HitResource to the hit actor if it is an AResources_M. May fail but checks later
			AResources_M* HitResource = Cast<AResources_M>(HitResult.GetActor());

			// Stamina check
			if (Stamina > 5.0f)
			{
				// Checks if casting was successful
				if (HitResource)
				{
					// Gets resource name and resource harvest amount, subtracts it from total, similar to health
					FString hitname = HitResource->resourceName;
					int resourceValue = HitResource->resourceAmount;

					HitResource->totalResource = HitResource->totalResource - resourceValue;

					// Checks if resource is depleted. If not, it gives the resource, spawns a decal, and reduces stamina. If so, it destroys the resource
					if (HitResource->totalResource > resourceValue)
					{
						GiveResource(resourceValue, hitname);

						// Updates objectives widget
						matsCollected = matsCollected + resourceValue;
						objWidget->UpdatematObj(matsCollected);


						UGameplayStatics::SpawnDecalAtLocation(GetWorld(), hitDecal, FVector(10.0f, 10.0f, 10.0f), HitResult.Location, FRotator(-90, 0, 0), 2.0f);

						SetStamina(-5.0f);
					}
					else
					{
						HitResource->Destroy();
					}
				}
			}
		}
	}

	else
	{
		isBuilding = false;

		// Updates objectives widget
		objectsBuilt = objectsBuilt + 1.0f;
		objWidget->UpdatebuildObj(objectsBuilt);
	}
	
}

// Functions to change health, hunger, and stamina by any amount when called, if it does not exceed the maximum. Will set stat to max if it would exceed
void APlayerChar::SetHealth(float amount)
{
	if (Health + amount <= 100)
	{
		Health = Health + amount;
	}
	else
	{
		Health = 100;
	}
}

void APlayerChar::SetHunger(float amount)
{
	if (Hunger + amount <= 100)
	{
		Hunger = Hunger + amount;
	}
	else
	{
		Hunger = 100;
	}
}

void APlayerChar::SetStamina(float amount)
{
	if (Stamina + amount <= 100)
	{
		Stamina = Stamina + amount;
	}
	else
	{
		Stamina = 100;
	}
}

// Function that is called by the timer. Decreases hunger, until it reaches 0, and then decreases health
// Temporarily increases stamina by 10 per second no matter what
void APlayerChar::DecreaseStats()
{
	if (Hunger > 0)
	{
		SetHunger(-1.0f);
	}

	SetStamina(10.0f);

	if (Hunger <= 0)
	{
		SetHealth(-3.0f);
	}
}

// Function to give resources. Adds amount to the resource array
void APlayerChar::GiveResource(float amount, FString resourceType)
{
	if (resourceType == "Wood")
	{
		ResourcesArray[0] = ResourcesArray[0] + amount;
	}
	
	if (resourceType == "Stone")
	{
		ResourcesArray[1] = ResourcesArray[1] + amount;
	}

	if (resourceType == "Berry")
	{
		ResourcesArray[2] = ResourcesArray[2] + amount;
	}
}


// Subtracts inputted amount of resources from array and adds to respective element in building pieces array. Called by macro in Crafting_W Blueprint
void APlayerChar::UpdateResources(float woodAmount, float stoneAmount, FString buildingObject)
{
	if (woodAmount <= ResourcesArray[0])
	{
		if (stoneAmount <= ResourcesArray[1])
		{
			ResourcesArray[0] = ResourcesArray[0] - woodAmount;
			ResourcesArray[1] = ResourcesArray[1] - stoneAmount;

			if (buildingObject == "Wall")
			{
				BuildingArray[0] = BuildingArray[0] + 1;
			}

			if (buildingObject == "Floor")
			{
				BuildingArray[1] = BuildingArray[1] + 1;
			}

			if (buildingObject == "Ceiling")
			{
				BuildingArray[2] = BuildingArray[2] + 1;
			}

		}
	}
}

// If the player is not already building, and the piece is available, it sets up spawn params, removes the piece from the array, and spawns the piece. Tick constanly updates location until the player interacts
void APlayerChar::SpawnBuilding(int buildingID, bool& isSuccess)
{
	if (!isBuilding)
	{
		if (BuildingArray[buildingID] >= 1)
		{
			isBuilding = true;
			FActorSpawnParameters SpawnParams;
			FVector StartLocation = PlayerCamComp->GetComponentLocation();
			FVector Direction = PlayerCamComp->GetForwardVector() * 400.0f;
			FVector EndLocation = StartLocation + Direction;
			FRotator myRot(0, 0, 0);

			BuildingArray[buildingID] = BuildingArray[buildingID] - 1;

			spawnedPart = GetWorld()->SpawnActor<ABuildingPart>(BuildPartClass, EndLocation, myRot, SpawnParams);

			isSuccess = true;
		}

		// Fixed issue with widget not closing. Recommended by Skye Wiggins in 3-1
		else
		{
			isSuccess = false;
		}
	}
}

// Rotates the spawned part by 90 degrees while placing a piece
void APlayerChar::RotateBuilding()
{
	if (isBuilding)
	{
		spawnedPart->AddActorWorldRotation(FRotator(0, 90, 0));
	}
}

