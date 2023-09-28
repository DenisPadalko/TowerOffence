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
}

void ATowerPawn::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	
	SetCollisionSphereRadius();
}

void ATowerPawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	const TObjectPtr<AActor> Player = GetClosestPlayer();
	if(Player)
	{
		const float Distance = GetDistanceTo(Player);
		if(Distance && Distance <= CollisionSphereRadius)
		{
			FRotator Rotation(0.0f);
			GetRotation(Player, &Rotation);
			TurnTurret(Rotation);
			if(TimeAfterLastShot <= 0.0f)
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
}

void ATowerPawn::GetRotation(const TObjectPtr<AActor> Player, FRotator* Rotation) const
{
	*Rotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), Player->GetActorLocation());
	Rotation->Roll = 0.0f;
	Rotation->Pitch = 0.0f;
	Rotation->Yaw -= 90.0f;
}

TObjectPtr<AActor> ATowerPawn::GetClosestPlayer() const
{
	if(!PlayerRef.IsEmpty())
	{
		TObjectPtr<AActor> ClosestPlayer = PlayerRef[0];
		for(int i = 1; i < PlayerRef.Num(); ++i)
		{
			if(GetDistanceTo(PlayerRef[i]) < GetDistanceTo(ClosestPlayer))
			{
				ClosestPlayer = PlayerRef[i];
			}
		}
		return ClosestPlayer;
	}
	return nullptr;
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
	UKismetSystemLibrary::SphereOverlapActors(CollisionSphere, CollisionSphere->GetComponentLocation(),
		CollisionSphereRadius, TraceObjectTypes, nullptr, IgnoreActors, PlayerRef);
}

void ATowerPawn::Fire()
{
	UE_LOG(LogTemp, Warning, TEXT("ATowerPawn::Fire() was called"));
}

