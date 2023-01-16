// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

class ASpline;

#include "Spline.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ZombieGroup.generated.h"

UCLASS()
class DEMOPROJECT_API AZombieGroup : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AZombieGroup();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	TArray<AActor*> m_pZombies;
	AActor* m_pGroupLeader;
	FColor m_Color;
	ASpline* m_pFormation;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void InitializeGroup(TArray<AActor*> selectedZombies, AActor* groupLeader);

	void SetColor(FColor color);
	void SetFormation(ASpline* pSpline);

	UFUNCTION(BlueprintCallable)
	void MoveGroupIntoFormation();
};
