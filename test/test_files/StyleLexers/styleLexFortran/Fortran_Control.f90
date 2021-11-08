! An example of reading a simple control with labels.
!
! Jason Blevins <jrblevin@sdf.lonestar.org>
! Durham, May 6, 2008

program control_file
  implicit none

  ! Input related variables
  character(len=100) :: buffer, label
  integer :: pos
  integer, parameter :: fh = 15
  integer :: ios = 0
  integer :: line = 0

  ! Control file variables
  real :: pi
  integer, dimension(5) :: vector

  open(fh, file='control_file.txt')

  ! ios is negative if an end of record condition is encountered or if
  ! an endfile condition was detected.  It is positive if an error was
  ! detected.  ios is zero otherwise.

  do while (ios == 0)
     read(fh, '(A)', iostat=ios) buffer
     if (ios == 0) then
        line = line + 1

        ! Find the first instance of whitespace.  Split label and data.
        pos = scan(buffer, ' 	')
        label = buffer(1:pos)
        buffer = buffer(pos+1:)

        select case (label)
        case ('pi')
           read(buffer, *, iostat=ios) pi
           print *, 'Read pi: ', pi
        case ('vector')
           read(buffer, *, iostat=ios) vector
           print *, 'Read vector: ', vector
        case default
           print *, 'Skipping invalid label at line', line
        end select
     end if
  end do

end program control_file
