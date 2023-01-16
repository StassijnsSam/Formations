// Fill out your copyright notice in the Description page of Project Settings.


#include "ZombieGroup.h"

// Sets default values
AZombieGroup::AZombieGroup()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AZombieGroup::BeginPlay()
{
	Super::BeginPlay();
	
}

void AZombieGroup::SetColor(FColor color)
{
	m_Color = color;
}

void AZombieGroup::SetFormation(ASpline* pSpline)
{
	m_pFormation = pSpline;
}

void AZombieGroup::MoveGroupIntoFormation()
{
	TArray<FVector> formationLocations = m_pFormation->GetPoinstAlongSpline(m_pZombies.Num());

	//send every zombie to the location that is closest to them
	for (AActor* pZombie : m_pZombies) {
		FVector closestLocation = formationLocations[0];
		float closestDistance = FVector::Distance(pZombie->GetActorLocation(), closestLocation);
		//start this loop from 1 since you already put in 0
		for (int i{ 1 }; i < formationLocations.Num(); ++i) {
			float distanceToLocation = FVector::Distance(pZombie->GetActorLocation(), formationLocations[i]);
			if (distanceToLocation < closestDistance) {
				closestDistance = distanceToLocation;
				closestLocation = formationLocations[i];
			}
		}
	}
}

// Called every frame
void AZombieGroup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AZombieGroup::InitializeGroup(TArray<AActor*> selectedZombies, AActor* groupLeader)
{
	m_pZombies = selectedZombies;
	m_pGroupLeader = groupLeader;
}

