/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     | Website:  https://openfoam.org
    \\  /    A nd           | Copyright (C) 2015-2021 OpenFOAM Foundation
     \\/     M anipulation  |
-------------------------------------------------------------------------------
License
    This file is part of OpenFOAM.

    OpenFOAM is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    OpenFOAM is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with OpenFOAM.  If not, see <http://www.gnu.org/licenses/>.

\*---------------------------------------------------------------------------*/

#include "interfaceCompositionModel.H"
#include "phaseModel.H"
#include "phasePair.H"
#include "phaseSystem.H"
#include "rhoReactionThermo.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
    defineTypeNameAndDebug(interfaceCompositionModel, 0);
    defineRunTimeSelectionTable(interfaceCompositionModel, dictionary);
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::interfaceCompositionModel::interfaceCompositionModel
(
    const dictionary& dict,
    const phasePair& pair
)
:
    pair_(pair),
    species_(dict.lookup("species")),
    Le_("Le", dimless, dict),
    phase_
    (
        // !!! This is a hack, to let the models know which side of the
        // interface it applies to. In general, we are going to need improve
        // the phase-pair system to take into account model sidedness and other
        // types of asymmetry from just dispersed/continuous.
        pair.phase1().name() == IOobject::group(dict.dictName())
      ? pair.phase1()
      : pair.phase2()
    ),
    otherPhase_(pair.otherPhase(phase_)),
    thermo_
    (
        phase_.mesh().lookupObject<rhoReactionThermo>
        (
            IOobject::groupName(physicalProperties::typeName, phase_.name())
        )
    ),
    otherThermo_
    (
        otherPhase_.mesh().lookupObject<rhoThermo>
        (
            IOobject::groupName
            (
                physicalProperties::typeName,
                otherPhase_.name()
            )
        )
    )
{}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

Foam::interfaceCompositionModel::~interfaceCompositionModel()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

Foam::tmp<Foam::volScalarField> Foam::interfaceCompositionModel::dY
(
    const word& speciesName,
    const volScalarField& Tf
) const
{
    const label speciei = composition().species()[speciesName];

    return Yf(speciesName, Tf) - composition().Y()[speciei];
}


Foam::tmp<Foam::volScalarField> Foam::interfaceCompositionModel::dYfPrime
(
    const word& speciesName,
    const volScalarField& Tf
) const
{
    return YfPrime(speciesName, Tf);
}


Foam::tmp<Foam::volScalarField> Foam::interfaceCompositionModel::D
(
    const word& speciesName
) const
{
    const label speciei = composition().species()[speciesName];
    const volScalarField& p(thermo_.p());
    const volScalarField& T(thermo_.T());

    return volScalarField::New
    (
        IOobject::groupName("D", pair_.name()),
        composition().kappa(speciei, p, T)
       /composition().Cp(speciei, p, T)
       /composition().rho(speciei, p, T)
       /Le_
    );
}


// ************************************************************************* //
