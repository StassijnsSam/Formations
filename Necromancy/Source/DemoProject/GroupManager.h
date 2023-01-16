// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
class AZombieGroup;

#include "ZombieGroup.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GroupManager.generated.h"

UCLASS()
class DEMOPROJECT_API AGroupManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGroupManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	int m_MaxGroupAmount{4};

	TArray<FColor> m_GroupColors{};
	TArray<AZombieGroup*> m_pGroups{};

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void AddGroup(AZombieGroup* pGroup);

};
