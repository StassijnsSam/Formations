// Fill out your copyright notice in the Description page of Project Settings.
#include "Spline.h"

// Sets default values
ASpline::ASpline()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	m_pSpline = CreateDefaultSubobject<USplineComponent>("Spline");
	m_pSpline->ClearSplinePoints();
	m_pSpline->bDrawDebug = true;
	if (m_pSpline)
	{
		SetRootComponent(m_pSpline);
	}
}

// Called when the game starts or when spawned
void ASpline::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ASpline::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ASpline::StartDrawing()
{
	GetWorldTimerManager().SetTimer(m_TimerHandle, this, &ASpline::AddSplinePoint, 0.2f, true, 0.0f);
}

void ASpline::StopDrawing()
{
	GetWorldTimerManager().ClearTimer(m_TimerHandle);
}

void ASpline::OnConstruction(const FTransform& Transform)
{
}

void ASpline::AddSplinePoint()
{
	auto playerController = GetWorld()->GetFirstPlayerController();
	
	//Get the mouse cursor and deproject down

	FHitResult hit;
	bool hasHit = playerController->GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, true, hit);

	if (hasHit) {
		//Check if the mouse is far enough from previous location
		FVector previousPointLocation = m_pSpline->GetLocationAtSplinePoint(m_pSpline->GetNumberOfSplinePoints(), ESplineCoordinateSpace::World);
		if (FVector::Distance(previousPointLocation, hit.Location) > m_MinDistanceBetweenSplinePoints) {
			m_pSpline->AddSplinePoint(hit.Location, ESplineCoordinateSpace::World, true);
		}
	}
}

