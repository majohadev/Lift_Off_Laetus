// Fill out your copyright notice in the Description page of Project Settings.


#include "CoreFragment.h"
#include "../GameManagement/Grid.h"
#include "../GameManagement/GridSpace.h"
#include "../GameManagement/LaetusGameMode.h"
#include "../Characters/CrewMember.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ACoreFragment::ACoreFragment() {
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	mesh = CreateDefaultSubobject<UStaticMeshComponent>("Mesh");
	mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);//Weird things happen with collision on, plus we don't need it anyways

	static ConstructorHelpers::FObjectFinder<UStaticMesh>CubeMeshAsset(TEXT("StaticMesh'/Game/Geometry/Meshes/TEAM_CoreFragment__2_.TEAM_CoreFragment__2_'"));
	mesh->SetStaticMesh(CubeMeshAsset.Object);
	//mesh->SetRelativeScale3D(FVector(1.f, 1.f, 1.f));
}

// Called when the game starts or when spawned
void ACoreFragment::BeginPlay() {
	Super::BeginPlay();

	gameMode = Cast<ALaetusGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if (gameMode) {
		grid = gameMode->getGameGrid();
	}
}

// Called every frame
void ACoreFragment::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);

}

void ACoreFragment::moveTo(AGridSpace* target, ACrewMember* pusher) {
	if (target == nullptr || target->isOccupied()) {
		return;
	}
	
	targetLocation = target;	
	newLocation = targetLocation->GetActorLocation() + FVector(0, 0, 20);
	oldLocation = gridSpace->GetActorLocation() + FVector(0, 0, 20);

	FVector2D unitDirection = grid->getUnitDifference(gridSpace, target);
	bool needsRotate = pusher->needToRotate(unitDirection);

	// Reset pointers/references. Do this now so the crew member pushing this
	// doesn't interpret this space as being occupied.
	setGridSpace(target);

	//Wait to move until the player has finished rotating (if they need to rotate)
	if (needsRotate) {
		FTimerHandle timerParams;
		GetWorld()->GetTimerManager().SetTimer(timerParams, this, &ACoreFragment::moveForward, 1.0f, false);
	}else {
		moveForward();
	}
}

void ACoreFragment::moveForward() {
	//Calculate how much to increment movement by in each iteration of the timer.
	
	moveIncrement = (newLocation - oldLocation) / 150;

	//Start the timer to increment the position up until we reach the destination
	GetWorld()->GetTimerManager().SetTimer(moveTimerHandle, this, &ACoreFragment::incrementMoveForward, 0.01, true);
}

void ACoreFragment::incrementMoveForward() {
	FVector currentLocation = GetActorLocation();
	float distance = FVector::Dist(currentLocation, newLocation);

	//If in a certain distance tolerance of the actual location, consider 
	//the movement completed. This handles cases where moveIncrement does 
	//not add up to exactly the destination location.
	if (FMath::Abs(distance) > 20) {
		//Destination has not been reached, increment position
		SetActorLocation(currentLocation + moveIncrement);
	}
	else {
		//Desination has been reached! Stop timer.
		SetActorLocation(newLocation);//Snap to the exact location
		GetWorld()->GetTimerManager().ClearTimer(moveTimerHandle);
	}
}

void ACoreFragment::setGridSpace(AGridSpace* space) {
	if (space && !space->isOccupied()) {

		if (gridSpace) {
			gridSpace->setOccupant(nullptr);
		}

		space->setOccupant(this);
		gridSpace = space;
	}
	UE_LOG(LogTemp, Warning, TEXT("TEMP"));
}

AGridSpace* ACoreFragment::getGridSpace() {
	return gridSpace;
}