(include "ast.scm")

(define (force-stream stream)
 (if (eq? stream '()) '() (cons (car stream) (force-stream (force (cdr stream))))))

(define (main)
 (write (force-stream (tokens (current-input-port)))))

(main)
