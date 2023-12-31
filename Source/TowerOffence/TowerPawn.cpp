// Fill out your copyright notice in the Description page of Project Settings.


#include "TowerPawn.h"

#include "Engine/OverlapInfo.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

ATowerPawn::ATowerPawn()
{
	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Collision sphere"));
	CollisionSphere->SetupAttachment(RootComponent);
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionSphere->SetVisibility(true);

	CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &ATowerPawn::OnBeginOverlap);
	CollisionSphere->OnComponentEndOverlap.AddDynamic(this, &ATowerPawn::OnEndOverlap);
}

void ATowerPawn::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	
	SetCollisionSphereRadius();
}

void ATowerPawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	const TObjectPtr<AActor> Player = GetClosestTarget();
	if(Player)
	{
		FRotator Rotation(0.0f);
		GetRotation(Player, &Rotation);
		TurnTurret(Rotation);
		if(Rotation.Equals(TurretMesh->GetComponentRotation(), RotationTolerance) && TimeAfterLastShot <= 0.0f)
		{
			TimeAfterLastShot = TimeBetweenShots;
			Fire();
		}
		else
		{
			TimeAfterLastShot -= GetWorld()->GetDeltaSeconds();
		}
	}
}

void ATowerPawn::GetRotation(const TObjectPtr<AActor> Player, FRotator* Rotation) const
{
	*Rotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), Player->GetActorLocation());
	Rotation->Roll = 0.0f;
	Rotation->Pitch = 0.0f;
	Rotation->Yaw -= 90.0f;
}

TObjectPtr<AActor> ATowerPawn::GetClosestTarget() const
{
	if(PlayerRef.IsEmpty())
	{
		return nullptr;
	}
	TObjectPtr<AActor> ClosestTarget = nullptr;
	float DistanceToClosestTarget = 0.0f;
	for(const TObjectPtr<AActor> Player : PlayerRef)
	{
		if(ClosestTarget == nullptr)
		{
			ClosestTarget = Player;
			DistanceToClosestTarget = GetSquaredDistanceTo(ClosestTarget);
		}
		const float Distance = GetSquaredDistanceTo(Player);
		if(Distance < DistanceToClosestTarget)
		{
			ClosestTarget = Player;
			DistanceToClosestTarget = Distance;
		}
	}
	return ClosestTarget;
}

void ATowerPawn::SetCollisionSphereRadius() const
{
	CollisionSphere->SetSphereRadius(CollisionSphereRadius);
}

void ATowerPawn::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	TArray<TEnumAsByte<EObjectTypeQuery>> TraceObjectTypes;
	TraceObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_Pawn));
	TArray<AActor*> IgnoreActors;
	IgnoreActors.Init(this, 1);
	TArray<TObjectPtr<AActor>> OutActors;
	UKismetSystemLibrary::SphereOverlapActors(CollisionSphere, CollisionSphere->GetComponentLocation(),
		CollisionSphereRadius, TraceObjectTypes, nullptr, IgnoreActors, OutActors);
	for(const TObjectPtr<AActor> Actor : OutActors)
	{
		PlayerRef.Add(Actor);
	}
}

void ATowerPawn::OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int OtherBodyIndex)
{
	PlayerRef.Remove(OtherActor);
}

void ATowerPawn::Fire()
{
	UE_LOG(LogTemp, Warning, TEXT("ATowerPawn::Fire() was called"));
}

