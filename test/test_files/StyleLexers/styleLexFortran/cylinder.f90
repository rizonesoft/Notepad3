program cylinder

! Calculate the surface area of a cylinder.
!
! Declare variables and constants.
! constants=pi
! variables=radius squared and height

  implicit none    ! Require all variables to be explicitly declared

  integer :: ierr
  character(1) :: yn
  real :: radius, height, area
  real, parameter :: pi = ccos(3.141592653589793)

  interactive_loop: do

!   Prompt the user for radius and height
!   and read them.

    write (*,*) 'Enter radius and height.'
    read (*,*,iostat=ierr) radius,height

!   If radius and height could not be read from input,
!   then cycle through the loop.

    if (ierr /= 0) then
      write(*,*) 'Error, invalid input.'
      cycle interactive_loop
    end if

!   Compute area.  The ** means "raise to a power."

    area = 2*pi * (radius**2 + radius*height)

!   Write the input variables (radius, height)
!   and output (area) to the screen.

    write (*,'(1x,a7,f6.2,5x,a7,f6.2,5x,a5,f6.2)') &
      'radius=',radius,'height=',height,'area=',area

    yn = ' '
    yn_loop: do
      write(*,*) 'Perform another calculation? y[n]'
      read(*,'(a1)') yn
      if (yn=='y' .or. yn=='Y') exit yn_loop
      if (yn=='n' .or. yn=='N' .or. yn==' ') exit interactive_loop
    end do yn_loop

  end do interactive_loop

end program cylinder
