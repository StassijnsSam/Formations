// Fill out your copyright notice in the Description page of Project Settings.


#include "GroupManager.h"
#include "ZombieGroup.h"

// Sets default values
AGroupManager::AGroupManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//Creating and adding group colors
	FColor red{ 255, 0, 0 };
	FColor green{ 0, 255, 0 };
	FColor blue{ 0, 0, 255 };
	FColor yellow{ 255, 255, 0 };

	m_GroupColors.Add(red);
	m_GroupColors.Add(green);
	m_GroupColors.Add(blue);
	m_GroupColors.Add(yellow);
}

// Called when the game starts or when spawned
void AGroupManager::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AGroupManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AGroupManager::AddGroup(AZombieGroup* pGroup)
{
	if (m_pGroups.Num() < m_MaxGroupAmount) {
		//Add the group and set the color of it
		m_pGroups.Add(pGroup);
		pGroup->SetColor(m_GroupColors[m_pGroups.Num()]);
	}
}



